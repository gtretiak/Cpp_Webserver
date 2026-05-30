#ifndef ROUTER_HPP
#define ROUTER_HPP
#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "CgiRequestHandler.hpp"
#include "HttpRequest.hpp"

class	Router {
	private:
		std::string	config_;
	public:
		Router(const std::string &config);
		Router(const Router &An);
		Router	&operator=(const Router &An);
		RequestHandler	*resolve(const HttpRequest &req);
		//examine request
		//decide on type
		//routing to it
		~Router();
};

#endif
