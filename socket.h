/*
*/
#ifndef SOCKET_H
#define SOCKET_H

#include "helpers.h"
#include <openssl/ssl.h>

class Socket {
public:
	uint32_t IP;
	int sock; // socket handle
  SSL* ssl; // ssl handle
	struct hostent* remote; // structure used in DNS lookups
	struct sockaddr_in server; // structure for connecting to server
  bool using_ssl;

	char *buf; // current buffer
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer

	Socket();
	~Socket();

	bool lookup_dns(WebResource* resource);
	bool Connect();
  bool setup_ssl(const char* host, bool verbose);
	bool Close();
	void clear_buffer();
	bool read_response(size_t max_download_size);
	bool send_request(std::string request);
};

#endif // SOCKET_H
