#include "CgiRequestHandler.hpp"

CgiRequestHandler::CgiRequestHandler(){}
void	CgiRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	(void)req;
	(void)res;
	//Executing CGI script, filling the response, using MimeTypes AND CGI's output
}
CgiRequestHandler::~CgiRequestHandler() {}
