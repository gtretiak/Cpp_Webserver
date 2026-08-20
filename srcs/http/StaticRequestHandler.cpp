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
#include <sys/stat.h>
#include <vector>
#include <dirent.h>
#include <cerrno>

StaticRequestHandler::StaticRequestHandler() : _config(NULL),
											   _server(NULL),
											   _location(NULL)
{}
void StaticRequestHandler::setConfig(globalConfig *config)
{
	_config = config;
}

void StaticRequestHandler::setContext(serverConfig *server, locationConfig *location)
{
	_server = server;
	_location = location;
}

bool	StaticRequestHandler::hasIndexDirective() {
	if (!_location->_index.empty())
		return true;
	if (!_server->_index.empty())
		return true;
	return false;
}

std::string	StaticRequestHandler::readFile(const std::string &path) {
	int	fd = open(path.c_str(), O_RDONLY);

	std::cout << "\t\t ****** StaticRequestHandler::readFile(): Opening file: " << path << std::endl;
	if (fd < 0)
		throw HttpException(404, "File Not Found" + path);
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
	if (path.find("uploads") == std::string::npos)
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

std::string StaticRequestHandler::getRoot() const
{
	if (_location && !_location->_root.empty())
		return _location->_root;
	if (_server && !_server->_root.empty())
		return _server->_root;
	throw HttpException(500, "No root directive configured");
}

/// @brief The behavior of remove the location block match prefix is exclusively to webserv project.
/// @param root root directive, from server or location
/// @param path URI from request
/// @return root + path(URI stripped from matched location prefix)
std::string StaticRequestHandler::buildFilePath(const std::string &root, const std::string &path) const
{
	std::string	remainder;
	std::string	locPrefix = _location->_path;

	if (path.compare(0, locPrefix.size(), locPrefix) == 0)
		remainder = path.substr(locPrefix.size());
	else
		remainder = path;

	if (root.empty())
		throw HttpException(500, "Empty root directive");

	if (!root.empty() && root[root.size() - 1] == '/' && !remainder.empty() && remainder[0] == '/')
		return root + remainder.substr(1);

	if (!root.empty() && root[root.size() - 1] != '/' && !remainder.empty() && remainder[0] != '/')
		return root + "/" + remainder;

	return root + remainder;
}

bool StaticRequestHandler::isDirectory(const std::string &path) const
{
	struct stat st;

	if (stat(path.c_str(), &st) == -1)
		return false;
	return S_ISDIR(st.st_mode);
}

bool StaticRequestHandler::isRegularFile(const std::string &path) const
{
	struct stat st;

	if (stat(path.c_str(), &st) == -1)
		return false;
	return S_ISREG(st.st_mode);
}

std::string	StaticRequestHandler::searchIndexFiles( const std::string &dirPath ) const
{
	const std::vector<std::string>	*indexes;
	struct dirent					*entry;

	indexes = NULL;
	if (_location && !_location->_index.empty())
		indexes = &_location->_index;
	else if (_server && !_server->_index.empty())
		indexes = &_server->_index;

	if (indexes != NULL) {
		std::cout << "StaticRequestHandler::searchIndexFile(): Using index files from " << (_location ? "location" : "server") << std::endl;
		std::cout << "Configured index files: \t";
		for (size_t i = 0; i < indexes->size(); ++i)
		{
			std::cout << (*indexes)[i];
			if (i < indexes->size() - 1)
				std::cout << ", ";
		}
	}
	else {
		std::cout << "	StaticRequestHandler::searchIndexFiles(): No index files configured" << std::endl;
		return ("");
	}


	//open dirpath
	DIR	*dir = opendir(dirPath.c_str());
	if (!dir) {
		throw HttpException(500, "Internal Error: failed to open dirpath of index files");
	}

	//search for any match within the indexes.
	while ((entry = readdir(dir)) != NULL) {
		std::string	filename(entry->d_name);

		if (filename == "." || filename == "..")
			continue;
		for (std::vector<std::string>::const_iterator it = indexes->begin(); it != indexes->end(); ++it) {
			const std::string &pattern = *it;

			//wildcard suffix match
			if (!pattern.empty() && pattern[0] == '*') {
				std::string suffix = pattern.substr(1);//get extension after '*'

				if (filename.size() >= suffix.size() &&
					filename.compare(filename.size() - suffix.size(),
									suffix.size(), suffix) == 0)
				{
					closedir(dir);
					return dirPath + "/" + filename;
				}
			}
			//literal match
			if (filename == pattern) {
				closedir(dir);
				return std::string(dirPath + "/" + filename);
			}
		}
	}
	closedir(dir);
	return ("");
}

std::string StaticRequestHandler::resolveIndexFile(std::string &dirPath)
{
	std::string	candidate;

	candidate = searchIndexFiles(dirPath);
	//std::cout << "\t\tDEBUG: resolveIndexFile():candidate = " << candidate << std::endl;
	if (isRegularFile(candidate))
		return candidate;
	return "";
}

bool StaticRequestHandler::getAutoindex() const
{
	if (_location)
		return _location->_autoindex;
	if (_server)
		return _server->_autoindex;
	return false;
}

std::string StaticRequestHandler::generateAutoindexPage(const std::string &requestPath, const std::string &dirPath) const
{
	DIR *dir = opendir(dirPath.c_str());
	if (dir == NULL)
	{
		if (errno == EACCES)
			throw HttpException(403, "Directory listing forbidden");
		throw HttpException(404, "Directory not found");
	}

	std::ostringstream html;

	html << "<html><head><title>Index of " << requestPath << "</title></head><body>";
	html << "<h1>Index of " << requestPath << "</h1>";
	html << "<ul>";

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		if (name == ".")
			continue;

		std::string href = requestPath;
		if (href.empty())
			href = "/";
		if (href[href.size() - 1] != '/')
			href += "/";
		href += name;

		html << "<li><a href=\"" << href << "\">" << name << "</a></li>";
	}

	closedir(dir);

	html << "</ul></body></html>";
	return html.str();
}

void	StaticRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	std::string	path = req.getPath();
	std::string	method = req.getMethod();
	bool		isAllowed = true;

	if (method == "GET")
		isAllowed = this->_location->_allowed_methods.GET;
	else if (method == "POST")
		isAllowed = this->_location->_allowed_methods.POST;
	else if (method == "DELETE")
		isAllowed = this->_location->_allowed_methods.DELETE;
	else
		throw HttpException(405, "Method Not Allowed: " + method);
	if (!isAllowed)
		throw HttpException(405, "Method Not Allowed: " + method);

	std::string	extension = "UnknownByDefault";
	size_t		dotPos = path.find_last_of('.');

	if (dotPos != std::string::npos)
		extension = path.substr(dotPos);
	std::string	type = MimeTypes::getMimeType(extension);
	std::string root = getRoot();
	std::string filePath = buildFilePath(root, path);
	std::cout << "type(text/html?:[" << type << "],\npath:[" << path << "]\nfilepath:[" << filePath << "]" << std::endl;//for development purposes only; to be removed TODO
	res.setHeader("connection", "keep-alive");//if the connection is still open
	//check if the there is any header in the request that says otherwise
	res.setHeader("cache-control", "public, max-age=3600");

	if (method == "GET")
	{
		//std::cout << "********* GOT HERE DEBUG 1: filePath: " << filePath << std::endl;
		if (isDirectory(filePath))
		{
			//std::cout << "********* GOT HERE DEBUG 1.5: filePath: " << filePath << std::endl;
			std::string indexPath = resolveIndexFile(filePath);
			//std::cout << "\n********* GOT HERE DEBUG 1.75: indexPath: " << indexPath << std::endl;

			if (!indexPath.empty())
				filePath = indexPath;
			else if (getAutoindex())
			{
				std::string content = generateAutoindexPage(path, filePath);
				res.setStatus(200);
				res.setHeader("content-type", "text/html");
				res.setBody(content);
				return;
			}
			else {
				if (hasIndexDirective())
					throw (HttpException(404, "Not Found"));
				else
					throw HttpException(403, "Directory listing forbidden");
			}
		}

		extension = "UnknownByDefault";
		dotPos = filePath.find_last_of('.');
		if (dotPos != std::string::npos)
			extension = filePath.substr(dotPos);
		type = MimeTypes::getMimeType(extension);

		std::string content = readFile(filePath);
		res.setStatus(200);
		res.setHeader("content-type", type);
		res.setBody(content);
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
