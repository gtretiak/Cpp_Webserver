#include "Router.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include "HttpException.hpp"

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

serverConfig *Router::getCurrentServer()
{
	if (this->config_ == NULL)
		throw HttpException(500, "Router config is not set");
	if (this->CurrentConn_ == NULL)
		throw HttpException(500, "Router connection context is not set");
	if (this->CurrentConn_->serverIndex < 0)
		throw HttpException(500, "Invalid server index");

	size_t index = static_cast<size_t>(this->CurrentConn_->serverIndex);
	if (index >= this->config_->servers.size())
		throw HttpException(500, "Server index out of range");

	return (&this->config_->servers[index]);
}

bool Router::locationMatches(const std::string &locationPath, const std::string &requestPath) const
{
	if (locationPath.empty())
		return false;

	if (locationPath == "/")
		return true;

	if (requestPath.compare(0, locationPath.size(), locationPath) != 0)
		return false;

	return true;
}

locationConfig *Router::findBestLocation(serverConfig &server, const std::string &path)
{
	locationConfig	*best;
	size_t			bestLen;

	best = NULL;
	bestLen = 0;

	for (size_t i = 0; i < server._locations.size(); ++i)
	{
		std::string locationPath = server._locations[i]._path;

		//check for wildcard extension match
		if (locationPath[0] == '*') {
			std::string	suffix = locationPath.substr(1);

			if (path.length() >= suffix.length()) {
				size_t	start_pos = path.length() - suffix.length();

				if (path.compare(start_pos, suffix.length(), suffix) == 0) {
					return &server._locations[i];
				}
			}
			continue ;
		}

		//standard prefix match
		if (locationMatches(locationPath, path) && locationPath.size() > bestLen)
		{
			best = &server._locations[i];
			bestLen = locationPath.size();
		}
	}
	return best;
}

bool	Router::isCgiRequest( std::string& target ) {
	std::map<std::string, cgiExecutableConf>::iterator	it;
	std::string		extension;
	size_t			dotPos;

	dotPos = target.find_last_of(".");
	if (dotPos == std::string::npos)
		return false;
	extension = target.substr(dotPos);
	if (CurrentConn_->matchedLocation->_has_cgi == true) {

		it = CurrentConn_->matchedLocation->_cgi.cgi_extension.find(extension);
		
		if (it != CurrentConn_->matchedLocation->_cgi.cgi_extension.end()) {
			std::string	req_method = CurrentConn_->req.getMethod();

			if (req_method == "GET" && it->second.allowedMethodsCGI.GET == false)
				return false;
			if (req_method == "POST" && it->second.allowedMethodsCGI.POST == false)
				return false;
			if (req_method == "DELETE" && it->second.allowedMethodsCGI.DELETE == false)
				return false;

			CurrentConn_->cgiExecutable = it->second.path;
			return true;
		}
	}
	if (CurrentConn_->matchedServer->_has_cgi == true) {

		it = CurrentConn_->matchedServer->_cgi.cgi_extension.find(extension);
		
		if (it != CurrentConn_->matchedServer->_cgi.cgi_extension.end()) {
			std::string	req_method = CurrentConn_->req.getMethod();

			if (req_method == "GET" && it->second.allowedMethodsCGI.GET == false)
				return false;
			if (req_method == "POST" && it->second.allowedMethodsCGI.POST == false)
				return false;
			if (req_method == "DELETE" && it->second.allowedMethodsCGI.DELETE == false)
				return false;
			
			CurrentConn_->cgiExecutable = it->second.path;
			return true;
		}
	}
	return false;
}

bool Router::hasReturnDirective(serverConfig *server, locationConfig *location) const
{
	if (location && location->_has_return)
		return true;
	if (server && server->_has_return)
		return true;
	return false;
}

int Router::applyReturnDirective(serverConfig *server, locationConfig *location, HttpResponse &res) const
{
	const std::map<int, std::string> *returnMap = NULL;

	if (location && location->_has_return)
		returnMap = &location->_return;
	else if (server && server->_has_return)
		returnMap = &server->_return;

	if (returnMap == NULL || returnMap->empty())
		return 1;

	int code = returnMap->begin()->first;
	std::string value = returnMap->begin()->second;

	std::cout << "******** Router::applyReturnDirective(): Applying return directive: code = " << code << ", value = " << value << std::endl;

	res.setStatus(code);
	res.setHeader("connection", "close");

	if (code >= 300 && code <= 308 && !value.empty())
	{
		res.setHeader("location", value);
		res.setBody("");
	}
	else
	{
		res.setHeader("content-type", "text/plain");
		res.setBody(value);
	}
	return 1;
}

int Router::resolve(HttpRequest &req, HttpResponse &res)
{
	std::string target;
	serverConfig *server;
	locationConfig *location;

	target = req.getUrl();
	server = getCurrentServer();
	location = findBestLocation(*server, req.getPath());
	CurrentConn_->matchedServer = server;
	CurrentConn_->matchedLocation = location;

	//std::cout << " **** RESOLVE(): target: " << target << std::endl;
	/*if (location)
		std::cout <<"\n**** RESOLVE(): matched location: " << location->_path << std::endl;
	else
		std::cout <<"\n**** RESOLVE(): no matching location" << std::endl;*/

	if (hasReturnDirective(server, location)) {
		//std::cout << " **** RESOLVE(): return directive found, applying return directive" << std::endl;
		return applyReturnDirective(server, location, res);
	}

	if (isCgiRequest(target))
	{
		CurrentConn_->req = req;
		cgiHandler.setServer(server);
		if (location)
			cgiHandler.setLocation(location);
		
		//std::cout << "\t**** RESOLVE(): CGI-BIN target: " << target << std::endl;
		cgiHandler.handleRequest(*CurrentConn_);
		res = CurrentConn_->res;

		//std::cout << "\n*************** printMetaVars() for current response *************** " << std::endl;
		//cgiHandler.printMetaVars();
		
		return (0);
	}
	else
	{
		std::cout << "\n ************** ENTERED STATIC REQUEST BLOCK (GET, POST DELETE)" << std::endl;
		staticHandler.setConfig(config_);
		staticHandler.setContext(server, location);
		staticHandler.handleRequest(req, res);
	}
	return (1);
}

CgiRequestHandler& Router::getCgiHandler( ) {
	return	cgiHandler;
}
Router::~Router() {}
