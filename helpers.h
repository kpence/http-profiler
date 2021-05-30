#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <sstream>

class WebResource {
public:
  std::string scheme;
  std::string host;
  int port;
  std::string path;
  std::string query;
  WebResource()
    : scheme()
    , host()
    , port(80)
    , path("/")
    , query()
  {}
};

class HttpRequest {
public:
	std::ostringstream request;
  WebResource* resource;
	HttpRequest(WebResource* rsc, const char* user_agent, bool robots = false)
    : resource(rsc)
  {
		if (!robots) {
			request << "GET " << rsc->path << rsc->query;
		}
		else {
			request << "GET /robots.txt";
		}
		request << " HTTP/1.0" << "\r\n";
		request << "User-agent: " << user_agent << "\r\n";
		request << "Host: " << rsc->host << "\r\n";
		request << "Connection: close" << "\r\n";
		request << "\r\n";
	}
	std::string get_string() {
		return request.str();
	}
};

std::string http_request(WebResource* rsc, const char* user_agent, bool robots);
double diffclock(clock_t clock1,clock_t clock2);

#endif //HELPERS_H
