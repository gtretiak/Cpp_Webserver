#include "StaticRequestHandler.hpp"
#include "MimeTypes.hpp"
#include <iostream>

StaticRequestHandler::StaticRequestHandler(){}
void	StaticRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	// filling the response: setBody(buf), setStatus(code), setVersion(version) - if needed
	std::string	extension = "UnknownByDefault";
	std::string	path = req.getPath();
	//std::string	content = readFile(path);TODO
	size_t	dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos)
		extension = path.substr(dotPos);
	res.setHeader("Content-Type", MimeTypes::getMimeType(extension));
	res.setHeader("Connection", "keep-alive");//if the connection is still open
	//toString()
}
StaticRequestHandler::~StaticRequestHandler() {}
