#ifndef ROUTER_HPP
#define ROUTER_HPP
#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include "HttpRequest.hpp"

class	Router {
	private:
		std::string	config_; //or globalConfig TODO
		StaticRequestHandler	staticHandler;
		CgiRequestHandler	cgiHandler;
	public:
		Router(const std::string &config);
		Router(const Router &An);
		Router	&operator=(const Router &An);
		RequestHandler	&resolve(const HttpRequest &req);//or route with req and res TODO
		//examine request
		//decide on type
		//routing to it
		~Router();
};

#endif
