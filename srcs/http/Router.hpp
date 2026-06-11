#ifndef ROUTER_HPP
#define ROUTER_HPP
#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include "HttpRequest.hpp"
#include "../config/globalConfig.hpp"

class	Router {
	private:
		globalConfig	config_;
		StaticRequestHandler	staticHandler;
		CgiRequestHandler	cgiHandler;
	public:
		Router(const globalConfig &config);
		Router(const Router &An);
		Router	&operator=(const Router &An);
		RequestHandler	&resolve(const HttpRequest &req);//or route with req and res TODO
		//examine request
		//decide on type
		//routing to it
		~Router();
};

#endif
