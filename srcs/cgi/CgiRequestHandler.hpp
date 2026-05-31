#ifndef CGIREQUESTHANDLER_HPP
#define CGIREQUESTHANDLER_HPP

#include "../http/RequestHandler.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include <string>

class	CgiRequestHandler : public RequestHandler {
	private:
		std::string	cgiDirectory_;
	public:
		CgiRequestHandler();
		CgiRequestHandler(const std::string &cgiDir);
		void	handleRequest(HttpRequest &req, HttpResponse &res);
		~CgiRequestHandler();
};

#endif
