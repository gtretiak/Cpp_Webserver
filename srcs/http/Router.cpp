#include "Router.hpp"

Router::Router(const std::string &config) : config_(config) {}
Router::Router(const Router &An) : config_(An.config_) {}
Router	&Router::operator=(const Router &An) {
	if (this != &An)
		this->config_ = An.config_;
	return (*this);
}
RequestHandler	&Router::resolve(const HttpRequest &req) {//req and res? TODO
	//examine request
	//decide on type
	//routing to it
	std::string	path = req.getPath();
	std::string	method = req.getMethod();//do I need this? TODO
	if (path.find("/cgi-bin") != std::string::npos)
		return (this->cgiHandler);//or simply call TODO
	// otherwise static. Hence to get location config with findLocation(path) TODO
	return (this->staticHandler);//same but with location TODO
}
Router::~Router() {
}
