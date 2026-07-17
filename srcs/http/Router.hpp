#ifndef ROUTER_HPP
#define ROUTER_HPP
#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "../config/globalConfig.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include "../server/Connection.hpp"
#include "HttpRequest.hpp"
#include "../config/globalConfig.hpp"

class	CgiRequestHandler;

class	Router {
	private:
		globalConfig*			config_;
		StaticRequestHandler	staticHandler;
		CgiRequestHandler		cgiHandler;
		Connection*				CurrentConn_;

		Router	&operator=(const Router &other);//do not use
	public:
		Router( );
		Router(globalConfig* config);
		Router(Router &other);
	
		void	resolve( HttpRequest &req, HttpResponse& res );

		void	setConfig( globalConfig* config);
		void	setConnEnv( Connection& conn );
		~Router();
};

#endif
