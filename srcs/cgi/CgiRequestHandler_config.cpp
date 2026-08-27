/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_config.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 17:54:34 by dopereir          #+#    #+#             */
/*   Updated: 2026/08/26 10:33:50 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"

void	CgiRequestHandler::insertStaticMetaVars( ) {
	_meta_vars["GATEWAY_INTERFACE"] = "CGI/1.1";
	_meta_vars["SERVER_SOFTWARE"] = "webserv/1.0";
}

/// @brief extracts the path from the URI, and removes the query string if present.
/// @brief example: if the URI is "/cgi-bin/script.py/extra/path?query=string", this function will return "/cgi-bin/script.py/extra/path".
/// @param requestUrl past the actual HTTP request line.
/// @param scriptFilename the actual script filename that was found in the requestUrl
/// @return 
std::string CgiRequestHandler::getPathInfo( std::string& requestUrl, const std::string& scriptFilename ) {
	std::string::size_type	queryPos = requestUrl.find('?');
	std::string				path = (queryPos != std::string::npos) ? requestUrl.substr(0, queryPos) : requestUrl;

	std::string::size_type	scriptPos = path.find(scriptFilename);
	if (scriptPos != std::string::npos) {
		std::string::size_type endOfScript = scriptPos + scriptFilename.length();
		if (endOfScript < path.length() && path[endOfScript] == '/') {
			return path.substr(endOfScript);
		}
	}
	return "";
}

/// @brief Find where the script name ends in the request URI, return everything up to and including the script name.
/// @brief This is used to set the SCRIPT_NAME meta variable.
/// @brief Example: If the request URI is "/cgi-bin/script.py/extra/path?query=string" and the script filename is "script.py", this function will return "/cgi-bin/script.py".
/// @param requestUrl past the actual HTTP request line.
/// @param scriptFilename the actual script filename that was found in the requestUrl
/// @return Fallback will just return the path without query string.
std::string CgiRequestHandler::getScriptName( std::string& requestUrl, const std::string& scriptFilename ) {
	std::string				path = requestUrl;
	std::string::size_type	queryPos = path.find('?');

	if (queryPos != std::string::npos)
		path = path.substr(0, queryPos);

	std::string::size_type	scriptPos = path.find(scriptFilename);

	if(scriptPos != std::string::npos)
		return path.substr(0, scriptPos + scriptFilename.length());
	return path;
}

std::string CgiRequestHandler::getPathFromURI(const std::string& URI) {
	if (URI.empty())
		return std::string();
	std::string::size_type	pos = URI.find('?');
	if (pos == std::string::npos)
		return URI;
	return URI.substr(0, pos);
}


std::string	CgiRequestHandler::getPathTranslated( const std::string& pathInfo ) {
	if (pathInfo.empty())
		return std::string();
	if (!_locSetting)
		return std::string();

	const std::string& root = !_locSetting->_root.empty()
		? _locSetting->_root : _serverSetting->_root;
	if (root.empty())
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

/// @brief This function sole responsibility is to find a location block
/// @brief in the .conf file that matches the target in the HTTP request
/// @brief line. It does not check if the CGI file exists, only check the
/// @brief folder that lives in.
/// @param server Server data set to search in.
/// @param pathTarget Target from the request line.
/// @return NULL if not find, and caller is expected to throw a 404 not found
/// @return , if finds return the locationConfig data set instance.
locationConfig*	CgiRequestHandler::findCgiLocation( serverConfig& server,
		const std::string& pathTarget) const
{
	locationConfig*	best = NULL;
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

std::string	CgiRequestHandler::httpHeaderToCgiMetaVar( const std::string& headerName ) {
	std::string result = "HTTP_";

	for (size_t i = 0; i < headerName.size(); ++i)
	{
		if (headerName[i] == '-')
			result += '_';
		else
			result += static_cast<char>(
				std::toupper(static_cast<unsigned char>(headerName[i])));
	}
	return result;
}

void	CgiRequestHandler::extractMetaVars( HttpRequest& req ) {
	const std::map<std::string, std::string>& headers = req.getHeaders();
	std::string	scriptUri;
	std::string	pathInfo;
	std::string	pathTranslated;

	if (!_serverSetting)
		throw (HttpException(500, "No server configuration available"));
	if (!_locSetting)
		std::cout << "\tNo location block configured for current cgi request" << std::endl;

	scriptUri = req.getPath();
	pathInfo = req.getPath();
	pathTranslated = buildCgiFilePath(getExecRoot(), req.getPath());

	insertStaticMetaVars( );

	_meta_vars["SERVER_PROTOCOL"] = req.getVersion();
	_meta_vars["REQUEST_METHOD"] = req.getMethod();

	_meta_vars["REQUEST_URI"] = req.getUrl();
	_meta_vars["QUERY_STRING"] = req.getQuery();
	
	_meta_vars["SCRIPT_NAME"] = scriptUri;
	_meta_vars["PATH_INFO"] = pathInfo;
	
	if (!pathInfo.empty())
		_meta_vars["PATH_TRANSLATED"] = pathTranslated;
	else
		_meta_vars["PATH_TRANSLATED"] = "";
	
	if (req.getMethod() == "POST") {
		std::stringstream	ss;
		ss << req.getBody().size();
		
		_meta_vars["CONTENT_LENGTH"] = ss.str();
		if (req.hasHeader("Content-Type"))
			_meta_vars["CONTENT_TYPE"] = req.getHeader("Content-Type");
		else
			_meta_vars["CONTENT_TYPE"] = "";
	}
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		if (it->first == "content-length" || it->first == "content-type")
			continue;
		std::string cgiVarName = httpHeaderToCgiMetaVar(it->first);
		_meta_vars[cgiVarName] = it->second;
	}
	
	//std::cout << "\n*************** printMetaVars() DEBUG: *************** " << std::endl;
	//printMetaVars( );
	//std::cout << "\n*************** printMetaVars DEBUG (END): *************** " << std::endl;
	_envp = metaVarsToEnvp();
}
