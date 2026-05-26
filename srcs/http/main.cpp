#include <iostream>
#include "HttpParser.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "CgiRequestHandler.hpp"
#include <map>

struct	Connection {
	std::string	readBuffer;
};

int main() {
	StaticRequestHandler	handler;
	HttpParser	parser;
	HttpResponse	response;
	HttpRequest	req;
	
	std::string requests[] = {
		"POST /a/b/../c?x=1 HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"hello",
		
		"GET /index.html HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n", // valid
		
		"GET /../../etc/passwd HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n", // 403 - path escape attempt

		"GET /%2e%2e/%2e%2e/etc/passwd HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n", // 403 - url encoding
	
		"GET / HTTP/1.1\r\n"
		"\r\n", // 400 - missing host

		"PUT / HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"\r\n", // 405 - invalid method
	
		"GET /index.html\r\n"
		"Host: test.com\r\n"
		"\r\n" }; // 400 - broken line
	int	numTests = 7;
	for (int t = 0; t < numTests; t++)
	{
		std::cout << "==========================================\n"
			<< "Test " << t + 1 << "\nRAW REQUEST ->\n\n" << requests[t] << std::endl;
		HttpRequest	req;
		response = HttpResponse();
		try {
			if (!parser.isRequestComplete(requests[t]))
				throw HttpException(400, "Incomplete request");
			parser.parseRequest(requests[t], &req); // change raw to raw2, raw3, etc.
			std::cout << "\nPARSED SUCCESSFULLY ->\n" << std::endl;
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
			handler.handleRequest(req, response);
			std::cout << "\nRESPONSE ->\n\n" << response.toString() << std::endl;
		}
		catch (const HttpException &e) {
			std::cout << "Error: " << e.code() << " " << e.what() << std::endl;
		}
	}
	return 0;
}
