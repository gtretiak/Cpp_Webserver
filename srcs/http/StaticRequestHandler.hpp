#ifndef STATICREQUESTHANDLER_HPP
#define STATICREQUESTHANDLER_HPP

#include "RequestHandler.hpp"

class	StaticRequestHandler : public RequestHandler {
	public:
		StaticRequestHandler();
		void	handleRequest(HttpRequest &req, HttpResponse &res);
		~StaticRequestHandler();
};

#endif
