#ifndef ROUTER_HPP
#define ROUTER_HPP
#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "../config/globalConfig.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include "HttpRequest.hpp"

class	CgiRequestHandler;

class	Router {
	private:
		globalConfig*			config_;
		StaticRequestHandler	staticHandler;
		CgiRequestHandler		cgiHandler;

		Router	&operator=(const Router &other);//do not use
	public:
		Router( );
		Router(globalConfig* config);
		Router(Router &other);
	
		void	resolve( HttpRequest &req, HttpResponse& res );

		void	setConfig( globalConfig* config);
		//maybe function handler errors here? 
		//examine request
		//decide on type
		//routing to it
		~Router();
};

#endif
