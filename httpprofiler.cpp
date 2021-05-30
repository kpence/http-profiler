#include "pch.h"
#define MAX_ROBOTS_DOWNLOAD_SIZE 16384
#define MAX_DOWNLOAD_SIZE 2097152

bool compare_response_logs(LoggedResponse& a, LoggedResponse& b) {
  return (a.elapsed_time < b.elapsed_time);
}

HttpProfiler::HttpProfiler(Socket* _socket)
	: socket(_socket)
  , response_log(compare_response_logs)
{
}

void HttpProfiler::print_response_log_profile(int num_requests) {
  double fastestTime = response_log.top().elapsed_time;
  double slowestTime = fastestTime;
  double meanTime=0, medianTime=0;
  int numSuccessful=0, num3xxCode=0, num4xxCode=0;
  int smallestResponseLen = response_log.top().response_len;
  int largestResponseLen = smallestResponseLen;
  int numOtherFailed = 0;
  double sumTotalTime= 0;
  int originalLogSize = response_log.size();

  int i = 0;
  LoggedResponse it;
  while (!response_log.empty()) {
    memcpy(&it, &(response_log.top()), sizeof(LoggedResponse));
    response_log.pop();
    if (i == int((originalLogSize-1) / 2)) {
      if (response_log.size() % 2 == 0) {
        medianTime = (it.elapsed_time + response_log.top().elapsed_time)/2;
      } else {
        medianTime = it.elapsed_time;
      }
    }
    fastestTime = it.elapsed_time;
    sumTotalTime += it.elapsed_time;
    smallestResponseLen = (it.response_len < smallestResponseLen) ?
      it.response_len : smallestResponseLen;
    largestResponseLen = (it.response_len > largestResponseLen) ?
      it.response_len : largestResponseLen;
    ++i;
    if (it.successful) {
      ++numSuccessful;
    } else if (it.status_code[0] == '3') {
      ++num3xxCode;
    } else if (it.status_code[0] == '4') {
      ++num4xxCode;
    }
  }

  numOtherFailed = num_requests - numSuccessful - num3xxCode - num4xxCode;
  meanTime = sumTotalTime / i;

	std::cout << "\n--------------------------------------" << std::endl;
  std::cout << "# of Requests: " << num_requests << std::endl
    << "Slowest Time: " << slowestTime << " ms" << std::endl
    << "Fastest Time: " << fastestTime << " ms" << std::endl
    << "Mean Time: " << meanTime << " ms" << std::endl
    << "Median Time: " << medianTime << " ms" << std::endl
    << "Smallest Response (Size in Bytes): " << smallestResponseLen << std::endl
    << "Largest Response (Size in Bytes): " << largestResponseLen << std::endl
    << "Percentage of requests that succeeded: " << 100.0*(float(numSuccessful) / float(num_requests)) << "%\n"
    << "Number of Failed Request with Status Code: 3xx = " << num3xxCode
    << ", 4xx = " << num4xxCode
    << ", other = " << numOtherFailed
    << std::endl;
}

bool HttpProfiler::connect_and_download_page_n_times(HttpRequest* request, int num_requests, bool verbose=false) {
  /* Clear response log */
  response_log = std::priority_queue<LoggedResponse, std::vector<LoggedResponse>,
    decltype(&compare_response_logs)>(compare_response_logs);

  /* Connect the socket */
  clock_t time = clock();
  if (!socket->Connect()) {
    std::cout << "failed to connect\n";
    return false;
  }
  std::cout << " done in " << diffclock(clock(), time) << " ms" << std::endl;

  /* Set up ssl */
  if (request->resource->scheme == "https" && request->resource->port == 443) {
    if (!socket->setup_ssl(request->resource->host.c_str(), true)) {
      std::cout << "failed to set up ssl\n";
      return false;
    }
  }

	/* Download page */
  int i;
  int N = (num_requests == 0) ? 1 : num_requests; 
  for (i = 0; i < N; ++i) {
    download_page(request, MAX_DOWNLOAD_SIZE, verbose);
  }

  /* Close socket */
  if (!socket->Close()) {
    std::cout << "\nError: failed to close socket and ssl connection!\n";
    return false;
  }
  return true;
}

char HttpProfiler::download_page(HttpRequest* request, size_t max_download_size, bool verbose=true) {
  if (verbose)
    std::cout << "\t  Loading... ";

	clock_t time = clock();

	/* Send HTTP request over socket */
  int i;
  for (i = 0; i < 5; ++i) {
    if (!socket->send_request(request->get_string())) {
      if (verbose)
        std::cout << "failed sending request\n";
      if (i == 4)
        return false;
      if (!socket->Close())
        return false;
      if (!socket->Connect())
        return false;
      if (!socket->setup_ssl(request->resource->host.c_str(), verbose))
        return false;
    }
  }

	/* Read the HTTP response */
	if (!socket->read_response(max_download_size))
		return false;

	/* Get the response from socket */
  response = socket->buf;
	response_len = socket->curPos;

	if (strncmp(response, "HTTP/1.", 7) != 0) {
    if (verbose)
      std::cout << "failed with non-HTTP header" << std::endl;
		return false;
	}

  /* Add information about response to response_log */
  double elapsed_time = diffclock(clock(), time);
  log_response(elapsed_time);

  if (verbose) {
    std::cout << "done in " << elapsed_time << " ms with " << response_len << " bytes" << std::endl;

    /* Verify header status code */
    std::cout << "\t  Verifying header... status code "
      << response[9] << response[10] << response[11] << std::endl;
  }

  char status_code = response[9];

	/* Return the status code */
  return status_code;
}

void HttpProfiler::log_response(double elapsed_time) {
  struct LoggedResponse resp;
  resp.elapsed_time = elapsed_time;
  resp.response_len = response_len;
  memcpy(resp.status_code, &(response[9]), 3);
  if (resp.status_code[0] == '2')
    resp.successful = true;
  else
    resp.successful = false;
  response_log.push(resp);
}

void HttpProfiler::print_response() {
	/* Print response */
  if (response != NULL) {
    std::cout << "\n--------------------------------------" << std::endl;
    std::cout << response << std::endl;
  }
}

