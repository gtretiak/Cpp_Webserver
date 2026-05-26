#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class	RequestHandler {
	public:
		RequestHandler();
		virtual void	handleRequest(HttpRequest &req, HttpResponse &res) = 0;
		virtual ~RequestHandler();
};

#endif
