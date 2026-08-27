#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "RequestHandler.hpp"
#include "StaticRequestHandler.hpp"
#include "HttpRequest.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include "../server/Connection.hpp"
#include "../config/globalConfig.hpp"
#include "../config/serverConfig.hpp"
#include "../config/locationConfig.hpp"

class	CgiRequestHandler;

class	Router {
	private:
		globalConfig*			config_;
		StaticRequestHandler	staticHandler;
		CgiRequestHandler		cgiHandler;
		Connection*				CurrentConn_;

		serverConfig			*getCurrentServer();
		locationConfig			*findBestLocation(serverConfig &server, const std::string &path);
		bool					locationMatches(const std::string &locationPath, const std::string &requestPath) const;

		bool					isCgiRequest( std::string& target );

		bool	hasReturnDirective(serverConfig *server, locationConfig *location) const;
		int		applyReturnDirective(serverConfig *server, locationConfig *location, HttpResponse &res) const;
		
		Router	&operator=(const Router &other);//do not use
	
	public:
		Router( );
		Router(globalConfig* config);
		Router(Router &other);
	
		int	resolve( HttpRequest &req, HttpResponse& res );

		void	setConfig( globalConfig* config);
		void	setConnEnv( Connection& conn );

		CgiRequestHandler&	getCgiHandler( );
		~Router();
};

#endif
