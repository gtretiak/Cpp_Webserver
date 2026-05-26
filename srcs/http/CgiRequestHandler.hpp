#ifndef CGIREQUESTHANDLER_HPP
#define CGIREQUESTHANDLER_HPP

#include "RequestHandler.hpp"

class	CgiRequestHandler : public RequestHandler {
	public:
		CgiRequestHandler();
		void	handleRequest(HttpRequest &req, HttpResponse &res);
		~CgiRequestHandler();
};

#endif
