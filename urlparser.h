/*
*/
#ifndef URLPARSER_H
#define URLPARSER_H

#include "helpers.h"

class UrlParser {
public:
	WebResource resource;
	std::regex re;
	UrlParser();
	bool parse(const std::string& url_s);
};

#endif // URLPARSER_H
