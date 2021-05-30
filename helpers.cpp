#include "pch.h"

std::string http_request(WebResource* rsc, const char* user_agent, bool robots = false)
{
	std::ostringstream request;
	if (!robots) {
		request << "GET " << rsc->path << rsc->query;
	}
	else {
		request << "HEAD /robots.txt";
	}
	request << " HTTP/1.0" << "\r\n";
	request << "User-agent: " << user_agent << "\r\n";
	request << "Host: " << rsc->host << "\r\n";
	request << "Connection: close" << "\r\n";
	request << "\r\n";
	return request.str();
}

double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}
