#include "Router.hpp"

Router::Router(const std::string &config) : config_(config) {}
Router::Router(const Router &An) : config_(An.config_) {}
Router	&Router::operator=(const Router &An) {
	if (this != &An)
		this->config_ = An.config_;
	return (*this);
}
RequestHandler	*Router::resolve(const HttpRequest &req) {
	(void)req;
	RequestHandler *res = new StaticRequestHandler();
	//examine request
	//decide on type
	//routing to it
	return (res);
}
Router::~Router() {
}
