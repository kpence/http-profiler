#include "pch.h"
#define MAX_PORT_NO 65535
#define MIN_PORT_NO 1

UrlParser::UrlParser() {
	resource = WebResource();
	re = std::regex("^([\\w]+):\\/\\/([\\.\\-\\w]+)(:\\d*){0,1}(\\/[\\/\\.\\-\\w]*){0,1}(\\?[\\-\\w\\=\\&\\.\\/\\:]*){0,1}(#[\\-\\w]*){0,1}$");
}


bool UrlParser::parse(const std::string& url_s) {
	/* Gather resource data with Regex */
	std::smatch match;

	std::cout << "URL: " << url_s << std::endl;

	std::cout << "\t  Parsing URL... ";

	if (std::regex_search(url_s, match, re) == true) {
		/* Validate the data */

		/* Validate scheme is http */
		if (!(match.str(1) == "http" || match.str(1) == "https")) {
			std::cout << "Error, only http or https scheme is supported." << std::endl;
			return false;
		}
		resource.scheme = match.str(1);

		/* Set host */
		if (match.str(2).empty()) {
			std::cout << "Error, missing host." << std::endl;
			return false;
		}
		resource.host = match.str(2);

		/* Verify the port number and set it */
		if (!match.str(3).empty()) {
			std::string s = match.str(3).substr(1);

			/* Verify it represents a number */
			std::string::const_iterator it = s.begin();
			while (it != s.end() && std::isdigit(*it)) ++it;
			if (s.empty() || it != s.end()) {
				std::cout << "Error, invalid port" << std::endl;
				return false;
			}

			/* Validate the range port numbers */
			int port = std::stoi(s);
			if (port < MIN_PORT_NO || port > MAX_PORT_NO) {
				std::cout << "Error, port must be a valid number (0 < port < 999999)." << std::endl;
				return false;
			}
			resource.port = port;
		}
		else {
			resource.port = (resource.scheme == "https") ? 443 : 80;
		}

		/* Set path if not empty */
		if (!match.str(4).empty())
			resource.path = match.str(4);
		else
			resource.path = "/";

		/* Set query if not empty */
		if (!match.str(5).empty())
			resource.query = match.str(5);
		else
			resource.query = "";

	}
	else {
		std::cout << "failed to parse url" << std::endl;
		return false;
		// Exit with proper code
	}

	std::cout << "host " << resource.host;
	std::cout << ", port " << resource.port;
	//std::cout << ", request " << resource.path << resource.query;
	std::cout << std::endl;
	return true;
}

