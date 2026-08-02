#include "StaticRequestHandler.hpp"
#include "MimeTypes.hpp"
#include "HttpException.hpp"
#include "HttpUtils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

StaticRequestHandler::StaticRequestHandler(){}

std::string	StaticRequestHandler::readFile(const std::string &path) {
	int	fd = open(path.c_str(), O_RDONLY);
	if (fd < 0)
		throw HttpException(400, "File Not Found" + path);
	std::string	content;
	char	buf[BUFFER_SIZE];
	ssize_t	bytesRead = 1;
	while (bytesRead > 0)
	{
		bytesRead = read(fd, buf, sizeof(buf));
		if (bytesRead <= 0)
			break ;
		content.append(buf, bytesRead);
	}
	close(fd);
	if (bytesRead < 0)
		throw HttpException(500, "Reading File Error");
	return content;
}

void	StaticRequestHandler::writeFile(const std::string &path, const std::string &content) {
	int	fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
		throw HttpException(500, "Error Openning File For Writing");
	ssize_t	bytesWritten = write(fd, content.c_str(), content.length());
	int res = close(fd);
	if (bytesWritten == -1 || static_cast<size_t>(bytesWritten) != content.length())
		throw HttpException(500, "Writing File Error");
	if (res == -1)
		throw HttpException(500, "Error Closing File For Writing");
}
void	StaticRequestHandler::deleteFile(const std::string &path) {
	if (unlink(path.c_str()) == -1)
		throw HttpException(500, "Deleting File Error");
}
bool	StaticRequestHandler::fileExists(const std::string &path) {
	int	fd = open(path.c_str(), O_RDONLY);
	if (fd == -1)
		return false;
	close(fd);
	return (true);
}
bool	StaticRequestHandler::hasWDPermission(const std::string &path) {
	if (path.find("./www/uploads") == std::string::npos)
		return false;
	return true;
}

std::string	StaticRequestHandler::getUniquePath(const std::string &path) {
	for (int i = 1; i < 10000; i++)
	{
		std::stringstream	ss;
		ss << i;
		std::string candidate = path + "/file_" + ss.str();
		if (!fileExists(candidate))
			return candidate;
	}
	throw HttpException(500, "Couldn't generate unique filename");
}

std::string	StaticRequestHandler::getFileName(const std::string &path) {
	size_t	pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return path;
	return (path.substr(pos + 1));
}

void	StaticRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	std::string	path = req.getPath();
	std::string	method = req.getMethod();
	if (method != "GET" && method != "POST" && method != "DELETE")
		throw HttpException(405, "Method Not Allowed: " + method);
	std::string	extension = "UnknownByDefault";
	size_t	dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos)
		extension = path.substr(dotPos);
	std::string	type = MimeTypes::getMimeType(extension);
	std::string	root = "./www"; //fetch from globalConfig or serverConfig
	//to fetch root from location config depends on the existence of locationConfig instance and root directive available
/*	if (loc) // to add once we have location config (and as a parameter too) TODO
		root = loc->root;*/
	std::string	filePath = root + path;
	/*if (type == "text/html")//temporary if/else block, to be removed TODO
		filePath = root + "/html" + path;
	else
		filePath = root + "/application" + path;*/
//	std::cout << "type(text/html?:[" << type << "],\npath:[" << path << "]\nfilepath:[" << filePath << "]" << std::endl;//for development purposes only; to be removed TODO
	res.setHeader("connection", "keep-alive");//if the connection is still open
	res.setHeader("cache-control", "public, max-age=3600");
	if (method == "GET")
	{
		std::string	content = readFile(filePath);
		res.setStatus(200);
		res.setHeader("content-type", type);
		res.setBody(content);
//		std::cout << "[GET] Served: " << filePath << std::endl;//to be removed TODO
		std::cout << "\nRESPONSE ->\n";//cout - conn.writeBuffer equivalent TODO
//		std::cout << res.toString() << std::endl;//to test only, to be sent to writeBuffer TODO
		//system(("open " + filePath).c_str());//to test only, to be removed TODO
	}
	else if (method == "POST")
	{
		if (!hasWDPermission(filePath))
			throw HttpException(403, "Forbidden: No Write Permission");
		std::string	body = req.getBody();
		if (body.empty())
			throw HttpException(400, "Bad Request: Empty Body for POST");
		std::string	uniquePath = getUniquePath(filePath);
		writeFile(uniquePath, body);
		res.setStatus(201);
		res.setHeader("content-type", type);
		res.setHeader("location", "/uploads/" + getFileName(uniquePath));
		res.setBody("");
//		std::cout << "[POST] Created: " << uniquePath << std::endl;//to be removed TODO
	}
	else if (method == "DELETE")
	{
		if (!fileExists(filePath))
			throw HttpException(404, "File Not Found");
		if (!hasWDPermission(filePath))
			throw HttpException(403, "Forbidden: No Delete Permission");
		deleteFile(filePath);
		res.setStatus(204);
		res.setBody("");
//		std::cout << "[DELETE] Removed: " << filePath << std::endl;//to be removed TODO
	}
//	res.setHeader("set-cookie", "sessionId=abc123; Max-Age=3600");// should be moved from here
}
StaticRequestHandler::~StaticRequestHandler() {}
