/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_config.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 17:54:34 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/10 17:55:47 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"

void	CgiRequestHandler::insertStaticMetaVars( ) {
	_meta_vars["GATEWAY-INTERFACE"] = "CGI/1.1";
	_meta_vars["SERVER-SOFTWARE"] = "webserv/1.0";
}

/// @brief extract PATH_INFO meta variable from the HTTP request,
/// @brief PATH_INFO ALWAYS come before query.
/// @param requestUrl
/// @return 
std::string CgiRequestHandler::getPathInfo( std::string& requestUrl) {
	static const std::string	cgiExtensions[] = { ".py", ".sh", ".pl", ".php", ".go" };
	std::string					path;
	std::string::size_type		queryPos;

	path = requestUrl;
	queryPos = path.find('?');
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
	size_t					i;

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
		i++;
	}
	envp[i] = NULL;
	return envp;
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
	_meta_vars["PATH-TRANSLATED"] = getPathTranslated();
	for (; it != _CgiMetaVarsList.end(); ++it) {
		if (req.hasHeader(*it)) {
			std::string	key = *it;
			std::string	value = req.getHeader(*it);
			setMetaVar(key, value);
		}
	}
	_envp = metaVarsToEnvp();
}
