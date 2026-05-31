#include "StaticRequestHandler.hpp"
#include "MimeTypes.hpp"
#include "HttpException.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

static std::string	readFile(const std::string &path) {
	int	fd = open(path.c_str(), O_RDONLY);
/*	if (fd < 0)
		throw HttpException(400, "File Not Found");*/
	std::string	content;
	char	buf[4096];
	ssize_t	bytesRead = 1;
	while (bytesRead > 0)
	{
		bytesRead = read(fd, buf, sizeof(buf));
		if (bytesRead <= 0)
			break ;
		content.append(buf, bytesRead);
	}
	close(fd);
/*	if (bytesRead < 0)
		throw HttpException(500, "Internal Server Error");*/
	return content;
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
	res.setHeader("Set-Cookie", "sessionId=abc123; Max-Age=3600");
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
