
// This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "helpers.h"
#include <stdlib.h>
#include <getopt.h>
#define RELEASE_VERSION "1.1"

void print_usage_and_abort() {
  std::cout << "Usage: httprofile [--verbose] [--check-robots-txt] (--help | [--profile <Number of Requests>] --url <URL>)" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
  bool use_verbose = false;
  bool ignore_robots = true;
  int num_requests = 1;
  bool is_profiling = false;
  std::string url_s;
  int c;
  while (1) {
    static struct option long_options[] =
    {
      {"url",     required_argument, 0, 'u'},
      {"profile", required_argument, 0, 'p'},
      {"verbose", no_argument,       0, 'v'},
      {"help",    no_argument,       0, 'h'},
      {"check-robots-txt", no_argument, 0, 'r'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, ":u:p:hvr", long_options, &option_index);
    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
      case 'u':
        if (url_s.length() != 0) {
          std::cout << "More than one --url option was given.\n";
          print_usage_and_abort();
        }
        url_s = optarg;
        break;

      case 'p':
        if (is_profiling) {
          std::cout << "More than one --profile option was given.\n";
          print_usage_and_abort();
        }

        is_profiling = true;

        // Check if argument can be parsed as integer
        char* p;
        unsigned long num_requests_arg;
        num_requests_arg = strtol(optarg, &p, 10);
        if (errno != 0 || *p != '\0' || num_requests_arg == 0 || num_requests_arg > INT_MAX) {
          std::cout << "Invalid argument for --profile." << std::endl;
          print_usage_and_abort();
        }
        else {
          num_requests = num_requests_arg;
        }
        break;

      case 'v':
        use_verbose = true;
        break;

      case 'h':
        print_usage_and_abort();
        break;

      case 'r':
        ignore_robots = false;
        break;

      case '?':
        /* getopt_long already printed an error message. */
        print_usage_and_abort();
        break;

      default:
        print_usage_and_abort();
    }
  }

	/* Check if valid number of command line arguments */
	if (argc == 1 || url_s.length() == 0) {
    print_usage_and_abort();
	}

	/* Set the user agent name */
	std::ostringstream user_agent;
	user_agent << "httprofile/" << RELEASE_VERSION;

  /* Parse the url */
  UrlParser url_parser;
	if (!url_parser.parse(url_s))
		return -1;

	/* Perform DNS lookup */
	Socket socket = Socket();
	if (!socket.lookup_dns(&(url_parser.resource)))
		return -1;

  HttpProfiler httpProfiler(&socket);

  /* Crawl the robots.txt */
  if (!ignore_robots) {
    std::cout << "\t  Connecting on robots... ";
    HttpRequest robots_request(&(url_parser.resource), user_agent.str().c_str(), true);
    httpProfiler.connect_and_download_page_n_times(&robots_request, 1, true);

    // Check the status code
    if (httpProfiler.response_log.top().status_code[0] != '4') {
      // TODO parse the robots.txt if it exists
      // For now, it rejects a site if it even has a robots.txt (if --check-robots-txt option is set)
      return -1;
    }
  }

  /* Crawl the pages */
	std::cout << "\t* Connecting on page... ";
	HttpRequest request(&(url_parser.resource), user_agent.str().c_str(), false);
  httpProfiler.connect_and_download_page_n_times(&request, num_requests, use_verbose);

  // Print response
  if (!is_profiling) {
    httpProfiler.print_response();
    std::cout << "huh?\n";
  }
  else
    httpProfiler.print_response_log_profile(num_requests);

	return 0;
}
