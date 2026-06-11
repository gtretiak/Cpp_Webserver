#include <iostream>
#include <sstream>
#include <cstdlib>
#include <map>
#include <exception>
#include <string>

#include "server/Server.hpp"

#include "http/HttpParser.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpException.hpp"
#include "http/RequestHandler.hpp"
#include "http/StaticRequestHandler.hpp"
#include "cgi/CgiRequestHandler.hpp"
#include "http/Router.hpp"
#include "http/ErrorPageGenerator.hpp"
#include "server/Connection.hpp"

static int runHttpTests(const std::string &configFile)
{
	Router router(configFile);
	HttpParser parser;

	std::string requests[] = {
		"GET /index.html HTTP/1.1\r\n"
		"Host: test.com\r\n"
		"Transfer-Encoding: chunked\r\n" // hexadecimal
		"\r\n"
		"3\r\n" // thi
		"thi\r\n"
		"a\r\n" // s is a bod
		"s is a bod\r\n"
		"1\r\n"
		"y\r\n"
		"0\r\n\r\n", // valid

		"POST /b/../uploads?x=1 HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"hello", // valid

		"DELETE /b/../uploads/file_1?x=1 HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Length: 5\r\n"
		"\r\n"
		"hello", // valid

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
		"\r\n"}; // 400 - broken line
	int numTests = 7;
	for (int t = 0; t < numTests; t++)
	{
		std::cout << "==========================================\n"
				  << "Test " << t + 1 << "\n\nRAW REQUEST ->\n"
				  << requests[t] << std::endl;
		Connection conn; // should be able to handle vector<Connection>
		conn.readBuffer = requests[t];
		HttpRequest req;	   // arbitrary number per connection, all reusable
		HttpResponse response; // paired to each request, all reusable
		try
		{
			if (!parser.isRequestComplete(conn.readBuffer))
			{ // temp code. To be handled in server loop
				response.setStatus(400);
				throw HttpException(400, "Incomplete request");
			}
			parser.parseRequest(conn.readBuffer, &req); // to be called from the outside!
			conn.httpVersion = req.getVersion();
			response.setVersion(conn.httpVersion);
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
			// router.route(req, res) should be called from the outside instead of handlers:
			RequestHandler *handler = &(router.resolve(req)); // for test purposes, to be removed
			handler->handleRequest(req, response);			  // same
			req.setHeader("cookie", response.getHeader("set-cookie"));
			// here should be conn.writeBuffer = res.toString() to be called from the outside
		}
		catch (const HttpException &e)
		{ // block should be reviewed
			std::cerr << "Error: " << e.code() << " " << e.what() << std::endl;
			response.setStatus(e.code());
			if (conn.httpVersion.empty()) // never set due to invalid request's version
				response.setVersion("HTTP/1.1");
			response.setHeader("content-type", "text/html");
			response.setHeader("connection", "close");
			std::string content = ErrorPageGenerator::generate(e.code());
			response.setBody(content);
			std::ostringstream filePath;
			filePath << "./www/errors/" << e.code() << ".html"; // to test
																// system(("open " + filePath.str()).c_str());//to test only, to be removed
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 2 && argc != 3)
	{
		std::cerr << "Invalid number of arguments: ";
		std::cerr << "a config file should be present" << std::endl;
		return 1;
	}

	try
	{
		if (argc == 3 && std::string(argv[2]) == "--http-tests")
			return runHttpTests(argv[1]);

		Server server;

		server.setup(4242);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}