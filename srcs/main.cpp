#include <iostream>
#include <map>
#include "http/HttpParser.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpException.hpp"
#include "http/RequestHandler.hpp"
#include "http/StaticRequestHandler.hpp"
#include "http/CgiRequestHandler.hpp"
#include "http/Router.hpp"
#include "server/Connection.hpp"

int main(int argc, char **argv) {
	if (argc != 2)
	{
		std::cerr << "Invalid number of arguments: ";
		std::cerr << "a config file should be present" << std::endl;
		return 1;
	}
	Router		router(argv[1]);
	HttpParser	parser;
	HttpResponse	response;
	
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
			<< "Test " << t + 1 << "\n\nRAW REQUEST ->\n" << requests[t] << std::endl;
		HttpRequest	req;
		response = HttpResponse();
		try {
			if (!parser.isRequestComplete(requests[t]))
			{
				response.setStatus(400);
				throw HttpException(400, "Incomplete request");
			}
			parser.parseRequest(requests[t], &req); // change raw to raw2, raw3, etc.
			std::cout << "\nPARSING SUCCESS ->" << std::endl;
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
			std::cerr << "Error: " << e.code() << " " << e.what() << std::endl;
			response.setStatus(e.code());
		}
		if (response.getStatusCode() < 400)
		{
			RequestHandler	*handler = &(router.resolve(req));
			handler->handleRequest(req, response);
			std::cout << "\nRESPONSE ->\n" << response.toString() << std::endl;//writebuf equivalent
		}
	}
	return 0;
}
