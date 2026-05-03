#include <iostream>
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include <map>

struct	Connection {
	std::string	readBuffer;
};

int main() {
	HttpParser	parser;
	HttpRequest	req;
	
	std::string raw =
		"POST /a/b/../c?x=1 HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"hello";
	std::string raw2 =
		"GET /index.html HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n"; // valid
	std::string raw3 =
		"GET /../../etc/passwd HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n"; // 403 - path escape attempt
	std::string raw4 = 
		"GET /%2e%2e/%2e%2e/etc/passwd HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n"; // 403 - url encoding
	std::string raw5 = 
		"GET / HTTP/1.1\r\n"
		"\r\n"; // 400 - missing host
	std::string raw6 = 
		"PUT / HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n"; // 405 - invalid method
	std::string raw7 = 
		"GET /index.html\r\n"
		"Host: test.com\r\n"
		"\r\n"; // 400 - broken line
	try {
		parser.parseRequest(raw, &req); // change raw to raw2, raw3, etc.
		std::cout << "Method: " << req.getMethod() << std::endl;
		std::cout << "Path: " << req.getPath() << std::endl;
		std::cout << "Query: " << req.getQuery() << std::endl;
		std::cout << "Version: " << req.getVersion() << std::endl;
		std::cout << "Body: " << req.getBody() << std::endl;
		std::cout << "Headers:" << std::endl;
		const std::map<std::string, std::string> &m = req.getHeaders();
		for (std::map<std::string, std::string>::const_iterator it = m.begin();
				it != m.end(); ++it)
			std::cout << "  [" << it->first << "] = " << it->second << std::endl;
	}
	catch (const HttpException &e) {
		std::cout << "Error: " << e.code() << " " << e.what() << std::endl;
	}
	return 0;
}
