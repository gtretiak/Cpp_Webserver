#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"
#include <iostream>
#include <sstream>

std::set<std::string>& CgiRequestHandler::initCgiMetaVars() {
	static std::set<std::string>	vars;

	if (vars.empty()) {
		vars.insert("AUTH-TYPE");
		vars.insert("CONTENT-LENGTH");
		vars.insert("CONTENT-TYPE");
		vars.insert("GATEWAY-INTERFACE");
		vars.insert("PATH-INFO");
		vars.insert("PATH-TRANSLATED");
		vars.insert("QUERY-STRING");
		//vars.insert("REMOTE-ADDR");
		//vars.insert("REMOTE-HOST");
		//vars.insert("REMOTE-IDENT");
		//vars.insert("REMOTE-USER");
		vars.insert("REQUEST-METHOD");
		vars.insert("SCRIPT-NAME");
		//vars.insert("SERVER-NAME");
		//vars.insert("SERVER-PORT");
		vars.insert("SERVER-PROTOCOL");
		vars.insert("SERVER-SOFTWARE");
	}
	return vars;
}

std::set<std::string> CgiRequestHandler::_CgiMetaVarsList = CgiRequestHandler::initCgiMetaVars();

CgiRequestHandler::CgiRequestHandler() {
	_globalConfig = NULL;
	_serverSetting = NULL;
	_locSetting = NULL;
	_envp = NULL;
}

CgiRequestHandler::CgiRequestHandler( const globalConfig* config)
	: _globalConfig(config), _serverSetting(NULL), _locSetting(NULL), _envp(NULL) {}

void	CgiRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	(void)req;
	(void)res;
	//Extracting CGI script's name from request's path
	//Checking its existance and accessability (throw 404 if not found, 403 if not executable)
	//Setting envs, fork(), execve() it (throw 504 if timeout)
	//Capturing the output by parent, parsing it (throw 502 if script returned an error)
	//Filling the response with parsed data
	//Waiting for child
	throw HttpException(501, "CGI Not Implemented");
}
CgiRequestHandler::~CgiRequestHandler() {}

void	CgiRequestHandler::setMetaVar( std::string& key, std::string& value ) {
	_meta_vars.insert(std::pair<std::string, std::string>(key, value));
}

/// @brief extract PATH_INFO meta variable from the HTTP request
/// @param requestUrl PATH_INFO ALWAYS come before query.
/// @return 
std::string CgiRequestHandler::getPathInfo( std::string& requestUrl) {
	static const std::string	cgiExtensions[] = { ".py", ".sh", ".pl", ".php", ".go" };
	std::string				path = requestUrl;
	std::string::size_type	queryPos = path.find('?');

	if (queryPos != std::string::npos)
		path = path.substr(0, queryPos);
	if (path.empty())
		return std::string();

	std::string::size_type	matchPos = std::string::npos;
	std::string::size_type	matchEnd = std::string::npos;

	for (size_t i = 0; i < sizeof(cgiExtensions) / sizeof(cgiExtensions[0]); ++i) {
		std::string::size_type	pos = path.rfind(cgiExtensions[i]);

		if (pos == std::string::npos)
			continue;
		std::string::size_type	end = pos + cgiExtensions[i].size();

		if (end != path.size() && path[end] != '/')
			continue;
		if (matchPos == std::string::npos || pos > matchPos) {
			matchPos = pos;
			matchEnd = end;
		}
	}
	if (matchPos == std::string::npos || matchEnd >= path.size())
		return std::string();
	return path.substr(matchEnd);
}

std::string	CgiRequestHandler::getPathTranslated( ) {
	if (!_locSetting)
		return std::string();
	const std::string& root = !_locSetting->_root.empty()
		? _locSetting->_root : _serverSetting->_root;
	if (root.empty())
		return std::string();

	std::string key = "PATH-INFO";
	std::string pathInfo = getMetaVar(key);
	if (pathInfo.empty())
		return std::string();

	std::string	result = root;
	if (result.size() > 0 && result[result.size() - 1] == '/' && pathInfo.size() > 0 && pathInfo[0] == '/') {
		result += pathInfo.substr(1);
	} else if (result.size() > 0 && result[result.size() - 1] != '/' && pathInfo.size() > 0 && pathInfo[0] != '/') {
		result += '/';
		result += pathInfo;
	} else {
		result += pathInfo;
	}
	return result;
}

std::string	CgiRequestHandler::getMetaVar( std::string& key ) const {
	std::map<std::string, std::string>::const_iterator	it = _meta_vars.find(key);
	if (it != _meta_vars.end())
		return it->second;
	return std::string();
}

void	CgiRequestHandler::insertStaticMetaVars( ) {
	_meta_vars["GATEWAY-INTERFACE"] = "CGI/1.1";
	_meta_vars["SERVER-SOFTWARE"] = "webserv/1.0";
}

void	CgiRequestHandler::extractMetaVars( HttpRequest& req ) {
	std::set<std::string>::iterator	it = _CgiMetaVarsList.begin();
	std::string	tmp_url;

	getConfigSettings( req );
	insertStaticMetaVars( );
	_meta_vars["REQUEST-METHOD"] = req.getMethod();	//mandatory in Request Line
	_meta_vars["SCRIPT-NAME"] = req.getPath();		//mandatory in Request Line
	_meta_vars["QUERY-STRING"] = req.getQuery();	//mandatory in Request Line
	_meta_vars["SERVER-PROTOCOL"] = req.getVersion();//mandatory in Request Line
	tmp_url = req.getUrl();
	_meta_vars["PATH-INFO"] = getPathInfo( tmp_url );
	_meta_vars["PATH-TRANSALATED"] = getPathTranslated();
	for (; it != _CgiMetaVarsList.end(); ++it) {
		if (req.hasHeader(*it)) {
			std::string	key = *it;
			std::string	value = req.getHeader(*it);
			setMetaVar(key, value);
		}
	}
	_envp = metaVarsToEnvp();
}

void	CgiRequestHandler::getConfigSettings( HttpRequest& req ) {
	if (!_globalConfig) {
		throw HttpException(500, "CGI configuration unavailable");
	}
	std::vector<serverConfig>::const_iterator	it = _globalConfig->servers.begin();
	std::string									pathTarget = req.getPath();
	const locationConfig*						bestLoc = NULL;
	const serverConfig*							bestServer = NULL;
	
	for (; it != _globalConfig->servers.end(); ++it) {
		const locationConfig*	loc = this->findCgiLocation(*it, pathTarget);

		if (loc && (!bestLoc || loc->_path.size() > bestLoc->_path.size())) {
			bestLoc = loc;
			bestServer = &(*it);
		}
	}
	if (!bestLoc) {
		throw HttpException(404, "Not Found");
	}
	_locSetting = bestLoc;
	_serverSetting = bestServer;
}

/// @brief This function sole responsibility is to find a location block
/// @brief in the .conf file that matches the target in the HTTP request
/// @brief line. It does not check if the CGI file exists, only check the
/// @brief folder that lives in.
/// @param server Server data set to search in.
/// @param pathTarget Target from the request line.
/// @return NULL if not find, and caller is expected to throw a 404 not found
/// @return , if finds return the locationConfig data set instance.
const locationConfig* CgiRequestHandler::findCgiLocation( const serverConfig& server,
		const std::string& pathTarget) const
{
	const locationConfig*	best = NULL;
	size_t			i;

	for (i = 0; i < server._locations.size(); ++i) {
		const std::string&	locPath = server._locations[i]._path;

		if (server._locations[i]._has_cgi && pathTarget.compare(0, locPath.size(), locPath) == 0) {
			if (!best || locPath.size() > best->_path.size()) {
				best = &server._locations[i];
			}
		}
	}
	return	best;
}


char**	CgiRequestHandler::metaVarsToEnvp( ) {
	std::map<std::string, std::string>::const_iterator	it;
	std::string	entry;
	char**		envp;
	size_t		i;

	envp = new char*[_meta_vars.size() + 1];
	i = 0;
	for (it = _meta_vars.begin(); it != _meta_vars.end(); ++it) {
		entry = it->first + "=" + it->second;
		envp[i] = new char[entry.size() + 1];
		std::strcpy(envp[i], entry.c_str());
		//std::cout << "DEBUG: ENTRY: " << entry << std::endl;
		i++;
	}
	envp[i] = NULL;
	return envp;
}

void	CgiRequestHandler::cgiExecutor( HttpResponse& res ) {
	//use chdir to execute script at root repository
	//get script name to be executed
	//fork()
	//execve
	//handle errors
	//waitpid
	//free envp
}
















void	CgiRequestHandler::printEnvp( ) {
	if (!_envp)
		return ;
	for (int i = 0; _envp[i]; ++i) {
		std::cout << "_envp[" << i << "]:\t\t" << _envp[i] << std::endl;
	}
}

void	CgiRequestHandler::printMetaVars( ) {
	std::map<std::string, std::string>::iterator	it = _meta_vars.begin();

	for (; it != _meta_vars.end(); ++it) {
		std::cout << "Key: " << it->first << "\tValue: " << it->second << std::endl;
	}
}

void	printRequest( HttpRequest &req ) {
	const std::map<std::string, std::string>	head = req.getHeaders();

	std::map<std::string, std::string>::const_iterator	it;

	it = head.begin();
	for (; it != head.end(); ++it) {
		std::cout << "Key: " << it->first
			<< "\t\tValue: " << it->second << std::endl;
	}

	std::cout << "Method: " << req.getMethod() << std::endl;
	std::cout << "URL: " << req.getUrl() << std::endl;
	std::cout << "Path: " << req.getPath() << std::endl;
	std::cout << "Query: " << req.getQuery() << std::endl;
	std::cout << "Version: " << req.getVersion() << std::endl;
	std::cout << "Body: " << req.getBody() << std::endl;

}

void	buildRequest(HttpRequest& req) {
	req.setMethod("POST");
	req.setUrl("/cgi-bin/app.py/api/v1/list?limit=10");
	req.setPath("/cgi-bin/py_script.py");
	req.setQuery("foo=bar&mode=demo");
	req.setVersion("HTTP/1.1");
	req.setHeader("host", "localhost");
	req.setHeader("user-agent", "cgi-demo/1.0");
	req.setHeader("content-type", "application/x-www-form-urlencoded");
	req.setBody("name=webserv&lang=c++98");
	{
		std::ostringstream	contentLength;
		contentLength << req.getBody().size();
		req.setHeader("content-length", contentLength.str());
	}
	req.setHeader("connection", "keep-alive");
}
/*
void	buildRequestComplex(HttpRequest &req) {
	req.setMethod("POST");
	req.setUrl("/cgi-bin/form_handler.py?name=foo&age=3&debug=1");
	req.setPath("/cgi-bin/form_handler.py");
	req.setQuery("name=foo&age=3&debug=1");
	req.setVersion("HTTP/1.1");
	req.setHeader("host", "localhost:8080");
	req.setHeader("content-type", "application/x-www-form-urlencoded");
	req.setBody("name=foo&age=3&debug=1");
	{
		std::ostringstream	contentLength;
		contentLength << req.getBody().size();
		req.setHeader("content-length", contentLength.str());
	}
	req.setHeader("connection", "keep-alive");
}

void	buildRequestFullEnv(HttpRequest &req) {
	req.setMethod("POST");
	req.setUrl("/cgi-bin/script.py/extra/path?name=foo&age=3");
	req.setPath("/cgi-bin/script.py");
	req.setQuery("name=foo&age=3");
	req.setVersion("HTTP/1.1");
	req.setHeader("host", "127.0.0.1:8080");
	req.setHeader("content-type", "application/json");
	req.setHeader("content-length", "18");
	req.setHeader("user-agent", "webserv-client/1.0");
	req.setHeader("accept", "");
	req.setHeader("server", "webserv/1.0");
	req.setHeader("connection", "keep-alive");
	req.setBody("{\"hello\":\"world\"}");
}
*/
