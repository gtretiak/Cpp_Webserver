#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"

CgiRequestHandler::CgiRequestHandler() : cgiDirectory_(".www/cgi-bin") {}
CgiRequestHandler::CgiRequestHandler(const std::string &cgiDir) : cgiDirectory_(cgiDir) {
}
void	CgiRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	(void)req;
	(void)res;
	//Extracting CGI script's name from request's path
	//Checking its existance and accessability (throw 404 if not found, 403 if not executable)
	//Setting envs, fork(), execve() it (throw 504 if timeout)
	//Capturing the output by parent, parsing it (throw 502 if script returned an error)
	//Filling the response with parsed data
	//Waiting for child
	throw HttpException(501, "CGI Not Implemented");
}
CgiRequestHandler::~CgiRequestHandler() {}
