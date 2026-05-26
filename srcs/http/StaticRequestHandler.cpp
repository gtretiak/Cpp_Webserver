#include "StaticRequestHandler.hpp"

StaticRequestHandler::StaticRequestHandler(){}
void	StaticRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	(void)req;
	(void)res;
	// reading from the static file, filling the response, using MimeTypes
}
StaticRequestHandler::~StaticRequestHandler() {}
