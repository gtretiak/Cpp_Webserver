#include "Router.hpp"
#include "../cgi/CgiRequestHandler.hpp"

Router::Router( ) : config_(NULL) {}

Router::Router(globalConfig* config) : config_(config), cgiHandler(config) {}

Router::Router(Router &other) : config_(other.config_),
	staticHandler(other.staticHandler), cgiHandler(other.config_) {
}

void	Router::setConfig( globalConfig* config ) {
	this->config_ = config;
	this->cgiHandler.setConfig(config);
}

void	Router::resolve( HttpRequest &req, HttpResponse &res ) {
	std::string	target = req.getUrl();

	std::cout << " **** RESOLVE(): target: " << target << std::endl;
	if (target.find("/cgi-bin") != std::string::npos) {

		std::cout << " **** GOT HERE " << std::endl;
		cgiHandler.handleRequest(req, res);

		std::cout << "\n*************** printMetaVars() *************** " << std::endl;
		cgiHandler.printMetaVars();
		
		return ;
	}
	// otherwise static.
	//staticHandler.handleRequest(req, res);
	
	return ;
}

Router::~Router() {}
