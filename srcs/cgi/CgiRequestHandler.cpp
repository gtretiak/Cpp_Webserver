/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 18:19:23 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/12 23:14:54 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"
#include "../http/Router.hpp"

static std::string	toLowerString( std::string value ) {
	for (std::string::size_type i = 0; i < value.size(); ++i)
		value[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(value[i])));
	return value;
}

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

CgiRequestHandler::CgiRequestHandler( globalConfig* config)
	: _globalConfig(config), _serverSetting(NULL), _locSetting(NULL), _envp(NULL) {}

CgiRequestHandler::~CgiRequestHandler() {}

void	CgiRequestHandler::setMetaVar( std::string& key, std::string& value ) {
	_meta_vars.insert(std::pair<std::string, std::string>(key, value));
}

void	CgiRequestHandler::setConfig( globalConfig* config ) {
	_globalConfig = config;
}

std::string	CgiRequestHandler::getMetaVar( std::string& key ) const {
	std::map<std::string, std::string>::const_iterator	it = _meta_vars.find(key);

	if (it != _meta_vars.end())
		return it->second;
	return std::string();
}

std::string	CgiRequestHandler::getQueryFromURI( const std::string& URI ) {
	std::string::size_type	pos;

	pos = URI.find('?');
	if (pos != std::string::npos) {
		return URI.substr(pos + 1);
	}
	else
		return "";
}

void	CgiRequestHandler::extractHeader( std::string line, size_t colon, HttpResponse &res ) {
	std::string	key;
	std::string	value;

	key = toLowerString(line.substr(0, colon));
	value = line.substr(colon + 1);

	//std::cout << "DEBUG: AT EXTRACT HEADER: key: " << key << std::endl;
	while(!value.empty() && value[0] == ' ')
		value.erase(0, 1);
	if (key == "status") {
		int code = std::atoi(value.c_str());
		if (code < 100 || code > 599)
			throw HttpException(502, "Bad Gateway: invalid Status Code in CGI output");
		if (!res.hasHeader("status")) {
			res.setHeader(key, value);
			res.setStatus(code);
		}
	}
	else if (key == "content-type") {
		if (!res.hasHeader("content-type"))
			res.setHeader("content-type", value);
	}
	else if (key == "location") {
		if (!res.hasHeader("status") && res.hasStatusCode() == false) {
			//res.setHeader("status", std::string("302"));//IN DOCUMENT RESPONSE CAN CAUSE A MISMATCH IN STATUSCODE AND STATUS HEADER CODE
			res.setStatus(302);
		}
		if (!res.hasHeader("location")) {
			res.setHeader("location", value);
		}
			
	}
	else if (key == "content-length") {
		int	len = std::atoi(value.c_str());
		if (len < 0 || len > getClientMaxBodySize())
			throw HttpException(502, "Bad Gateway: content-lenght larger than configuration setting");
		// handled by setBody
	}
	else if (key == "set-cookie" || key == "cache-control" ||
			key == "expires" || key == "last-modified" ||
			key == "etag" || key == "www-authenticate" ||
			key == "pragma" || key == "content-language") {
		res.setHeader(key, value);
	}
	//still need date, server, connection and trasnfer-encoding

	//comment this line because need further cgi response classification to handle redirects
	/*if (!res.hasHeader("content-type"))
		throw HttpException(502, "Bad Gateway: CGI response missing content-type header");
	*/
}

void	CgiRequestHandler::parseHeaderSection( std::string& headerSection, HttpResponse& res ) {
	std::string	line;
	size_t		start;
	size_t		end;
	size_t		colon;

	start = 0;
	res.setStatus(200);
	while(start < headerSection.size()) {
		end = headerSection.find("\r\n", start);
		if (end == std::string::npos)
			end = headerSection.find("\n", start);
		if (end == std::string::npos)
			end = headerSection.size();

		line = headerSection.substr(start, end - start);
		start = end + 2;
		if (line.empty())
			break ;
		colon = line.find(":");
		if (colon == std::string::npos)
			throw HttpException(502, "Bad gateway: malformed CGI header");
		extractHeader(line, colon, res);
	}
}

void	CgiRequestHandler::parseCgiHttpResponse( HttpResponse &res, std::string &cgiOutput ) {
	size_t		headerEnd;

	headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		throw HttpException(502, "Bad gateway: no header terminator in CGI output");
	}
	std::string	headerSection = cgiOutput.substr(0, headerEnd);
	std::string	body = cgiOutput.substr(headerEnd + 4);
	{
		size_t	location;
		location = headerSection.find("status:", 0);
		if (location != std::string::npos)
			res.setStatusBool(true);
	}
	
	parseHeaderSection(headerSection, res);
	if (body.size() > 0) //issue when the resquest is of type CGI_DOCUMENT
		res.setBody(body);//SETS content-lenght automatically ISSUE!
}

void	CgiRequestHandler::handleRequest(HttpRequest &req, HttpResponse &res) {
	cgiResponseType	type;
	Router			internalRouter;
	HttpResponse	newRes;
	HttpRequest		newReq;

	extractMetaVars( req );
	cgiExecutor(req, res); //still need to handle timeout throw 504

	type = classifyCgiResponse(res);
	internalRouter.setConfig(_globalConfig);
	switch(type) {
		case CGI_DOCUMENT:
			std::cout << "@@@@@DEBUG: CGI DOCUMENT" << std::endl;
			break;
		case CGI_LOCAL_REDIR:
			newReq = processLocalRedir(res);
			newReq.setRedirectCount(req.getRedirectCount() + 1);
			
			std::cout << "******* PRINT INTERNAL REQUEST ******" << std::endl;
			printRequest(newReq);

			if (req.getRedirectCount() > 10)
				throw HttpException(508, "Loop Detected");
			
			std::cout << "****** INTERNAL RESOLVE ******" << std::endl;
			internalRouter.resolve(newReq, newRes);
			res = newRes;

			std::cout << "@@@@@DEBUG: CGI LOCAL REDIR" << std::endl;

			break;
		case CGI_CLIENT_REDIR:
			processClientRedir(res);
			std::cout << "@@@@@DEBUG: CGI CLIENT REDIR" << std::endl;
			break;
		case CGI_CLIENT_DOC_REDIR:
			processClientRedirWithDocument(res);
			std::cout << "@@@@@DEBUG: CGI CLIENT DOCUMENT REDIR" << std::endl;
			break;
		default:
			std::cout << "@@@@@DEBUG: NONE" << std::endl;
			break;
	}
	
	std::cout << "\n*************** handlerRequest()->printEnvp() *************** " << std::endl;
	printEnvp();
	if(_envp)
		freeEnvp( );
}
