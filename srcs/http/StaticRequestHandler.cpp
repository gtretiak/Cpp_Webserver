#include "StaticRequestHandler.hpp"
#include "MimeTypes.hpp"
#include "HttpException.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

static std::string	readFile(const std::string &path) {
	std::ifstream	file(path.c_str());
	/*if (!file.is_open())
		throw HttpException(400, "File Not Found"); OR should it be something else? TODO*/
	std::stringstream	buf;
	buf << file.rdbuf();
	return buf.str();
}

StaticRequestHandler::StaticRequestHandler(){}
void	StaticRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	std::string	path = req.getPath();
	std::string	extension = "UnknownByDefault";
	size_t	dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos)
		extension = path.substr(dotPos);
	std::string	type = MimeTypes::getMimeType(extension);
	res.setHeader("Content-Type", type);
	res.setHeader("Cache-Control", "public, max-age=3600");
	//'public' means by any cache (proxy, browser), 3600seconds = 1hour
	/*
	- For static files (images, CSS, JS):
	res.setHeader("Cache-Control", "public, max-age=31536000");  // 1 year
	- For HTML (revalidate frequently):
	res.setHeader("Cache-Control", "public, max-age=3600, must-revalidate");
	- For dynamic content (don't cache):
	res.setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	- For private data (only browser, not proxy):
	res.setHeader("Cache-Control", "private, max-age=1800");
	- Immutable (never changes):
	res.setHeader("Cache-Control", "public, max-age=31536000, immutable");*/
	std::string	filePath = "/var/www/" + type + path;
	std::string	content = readFile(filePath);
	res.setBody(content);
	//res.setHeader("Lat-Modified", formatTime(fileTime));TODO
	//res.setHeader("ETag", generateETag(content));TODO
	res.setHeader("Connection", "keep-alive");//if the connection is still open
	//setStatus(code) if needed
	//setVersion(version) - if needed
	std::cout << "\nRESPONSE ->\n";//cout - conn.writeBuffer equivalent
	std::cout << res.toString() << std::endl;
}
StaticRequestHandler::~StaticRequestHandler() {}
