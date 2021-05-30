#include "pch.h"
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define INITIAL_BUF_SIZE 8192
#define RESET_BUF_SIZE 32768
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

Socket::Socket()
  : allocatedSize(INITIAL_BUF_SIZE)
  , curPos(0)
  , buf(NULL)
  , IP()
  , remote()
  , server()
  , sock()
  , ssl(NULL)
  , using_ssl(false)
{
  signal(SIGPIPE, SIG_IGN);
}

Socket::~Socket()
{
  // Clean up buffer
  clear_buffer();
  free(buf);
  buf = NULL;
}

void Socket::clear_buffer() {
  if (buf != 0)
    buf[0] = '\0';
  // Set buffer's cursor pos to the start
  curPos = 0;
}

bool Socket::lookup_dns(WebResource* resource) {
  // first assume that the string is an IP address
  uint32_t IP = inet_addr(resource->host.c_str());
  if (IP == INADDR_NONE) {
    // if not a valid IP, then do a DNS lookup
    if ((remote = gethostbyname(resource->host.c_str())) == NULL) {
        std::cout << "failure in dns lookup\n";
        return false;
    }
    else {
      // take the first IP address and copy into sin_addr
      memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
    }
  }
  else {
    // if a valid IP, directly drop its binary version into sin_addr
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = IP;
  }

  // setup the port # and protocol type
  server.sin_family = AF_INET;
  server.sin_port = htons(resource->port); // host-to-network flips the byte order

  // create this buffer once
  if (buf == NULL)
    buf = (char*)malloc(INITIAL_BUF_SIZE * sizeof(char));

  return true;
}


bool Socket::Connect() {
  // Set cursor pos to the start
  clear_buffer();
  using_ssl = false; // internal flag, initally set to false by default.

  // open a TCP socket
  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock <= INVALID_SOCKET)
    return false;

  // Connect socket to server
	if (connect(sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) <= SOCKET_ERROR)
	{
    std::cout << "connection failed: " << strerror(errno) << " (" << (int)errno << ")\n";
    return false;
	}

  return true;
}


void log_ssl()
{
  int err;
  while ((err = ERR_get_error()) != 0) {
    char *str = ERR_error_string(err, 0);
    if (!str)
        return;
    printf("%s\n",str);
    fflush(stdout);
  }
}


bool Socket::setup_ssl(const char* host, bool verbose=true) {
  // set internal state
  using_ssl = true;

  // initalize the SSL library
  SSL_library_init();
  SSLeay_add_ssl_algorithms();
  SSL_load_error_strings();
  const SSL_METHOD *meth = TLS_client_method();
  SSL_CTX *ctx = SSL_CTX_new(meth);
  SSL_CTX_set_min_proto_version(ctx, 0);
  SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

  // initialize the ssl connection object
  ssl = SSL_new(ctx);
  if (!ssl) {
    std::cout << "error creating SSL.\n";
    log_ssl();
    return false;
  }
  SSL_set_fd(ssl, sock);

  SSL_set_min_proto_version(ssl, 0);
  SSL_set_max_proto_version(ssl, TLS1_3_VERSION);

  SSL_set_tlsext_host_name(ssl, host);

  // create ssl connection
  int err = SSL_connect(ssl);
  if (err <= 0) {
    printf("Error creating SSL connection.  err=%x\n", err);
    log_ssl();
    return false;
  }
  if (verbose)
    printf("\t  SSL connection using %s\n", SSL_get_cipher(ssl));
  return true;
}

bool Socket::Close() {
  // If buffer size is over 32 KB, reset it to INITIAL_BUF_SIZE
  if (allocatedSize > RESET_BUF_SIZE) {
    buf = (char*)realloc(buf, INITIAL_BUF_SIZE * sizeof(char));
    allocatedSize = INITIAL_BUF_SIZE;
  }

  if (using_ssl)
    SSL_free(ssl);

  using_ssl = false;

  if (close(sock) <= SOCKET_ERROR)
    return false;

  return true;
}


bool Socket::send_request(std::string request)
{
  // declare a write set
  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(sock, &wfds);

  struct timeval timeout, starttime, currtime;
  gettimeofday(&starttime, NULL);

  int curBytes = 0;
  int ret;

  while (true) {
    gettimeofday(&currtime, NULL);
    timeout.tv_sec = 10 - (currtime.tv_sec - starttime.tv_sec);
    timeout.tv_usec = 0;

    // wait to see if socket has any data
    if ((ret = select(sock+1, NULL, &wfds, NULL, &timeout)) > 0)
    {
      // able to send again; now send the next segment
      int bytes;
      if (using_ssl)
        bytes = SSL_write(ssl, request.c_str() + curBytes, request.length() - curBytes);
      else
        bytes = send(sock, request.c_str() + curBytes, request.length() - curBytes, 0);

      if (bytes < 0)
          break;

      if (bytes == 0 || bytes == request.length() - curBytes)
         return true; // normal completion

      curBytes += bytes;
    }
    else if (ret == 0) {
      std::cout << "failed with slow download\n";
      break;
    }
    else {
      break;
    }
  }
  return false;
}


bool Socket::read_response(size_t max_download_size)
{
  // declare a read set
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sock, &readfds);

  struct timeval timeout, starttime, currtime;
  gettimeofday(&starttime, NULL);

  int ret;

  while (true) {
    gettimeofday(&currtime, NULL);
    timeout.tv_sec = 10 - (currtime.tv_sec - starttime.tv_sec);
    timeout.tv_usec = 0;
    // wait to see if socket has any data
    if ((ret = select(sock+1, &readfds, NULL, NULL, &timeout)) > 0)
    {
      // new data available; now read the next segment
      int bytes;

      if (using_ssl)
        bytes = SSL_read(ssl, buf + curPos, allocatedSize - curPos);
      else
        bytes = recv(sock, buf + curPos, allocatedSize - curPos, 0);

      if (bytes < 0)
         break;

      if (bytes == 0) {
         *(buf + curPos) = '\0';
         return true; // normal completion
      }

      curPos += bytes; // adjust where the next recv goes

      // If larger than max download size
      if (curPos > max_download_size) {
         std::cout << "failed with exceeding max\n";
         break;
      }

      while (allocatedSize - curPos < (allocatedSize/2)) {
         allocatedSize *= 2;
         buf = (char*)realloc(buf, allocatedSize * sizeof(char));
      }

    }
    else if (ret == 0) {
      std::cout << "failed with slow download\n";
      break;
    }
    else {
      break;
    }
  }
  return false;
}

