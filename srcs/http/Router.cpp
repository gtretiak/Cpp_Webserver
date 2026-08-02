#include "Router.hpp"
#include "../cgi/CgiRequestHandler.hpp"

Router::Router( ) : config_(NULL), CurrentConn_(NULL) {}
Router::Router(globalConfig* config) : config_(config),
	cgiHandler(config),
	CurrentConn_(NULL) {}

Router::Router(Router &other) : config_(other.config_),
	staticHandler(other.staticHandler),
	cgiHandler(other.config_),
	CurrentConn_(NULL) {}

void	Router::setConfig(globalConfig* config) {
	this->config_ = config;
	this->cgiHandler.setConfig(config);
}
void	Router::setConnEnv(Connection& conn) {
	this->CurrentConn_ = &conn;
}
int	Router::resolve(HttpRequest &req, HttpResponse &res) {
	std::string	target = req.getUrl();
//	std::cout << " **** RESOLVE(): target: " << target << std::endl;TODO - remove before submit
	if (target.find("/cgi-bin") != std::string::npos) {
		CurrentConn_->req = req;
//		std::cout << " **** RESOLVE(): CGI-BIN target: " << target << std::endl;TODO - remove
		cgiHandler.handleRequest(*CurrentConn_);
		res = CurrentConn_->res;

//		std::cout << "\n*************** printMetaVars() for current response *************** " << std::endl;TODO - remove
		cgiHandler.printMetaVars();
		
		return (0);
	}
	else {
//		std::cout << "\n ************** ENTERED STATIC REQUEST BLOCK (GET, POST DELETE)" << std::endl;TODO - remove before submitting
		staticHandler.handleRequest(req, res);
		//return flag saying is static
	}
	return (1);
}
Router::~Router() {}
