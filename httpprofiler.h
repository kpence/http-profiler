/*
*/
#ifndef HTTPPROFILER_H
#define HTTPPROFILER_H

#include "pch.h"

struct LoggedResponse {
  char status_code[3];
  int response_len;
  double elapsed_time;
  bool successful;
};

bool compare_response_logs(LoggedResponse& a, LoggedResponse& b);

class HttpProfiler {
public:
	Socket* socket;
	clock_t time;
	char* response;
	int response_len;
  std::priority_queue<LoggedResponse, std::vector<LoggedResponse>,
    decltype(&compare_response_logs)> response_log;

	HttpProfiler(Socket* _socket);
	bool connect_and_download_page_n_times(HttpRequest* request, int num_requests, bool verbose);
	char download_page(HttpRequest* request, size_t max_download_size, bool verbose);
	void log_response(double elapsed_time);
	void print_response();
  void print_response_log_profile(int num_requests);
	bool verify_robots();
};

#endif // HTTPPROFILER_H
