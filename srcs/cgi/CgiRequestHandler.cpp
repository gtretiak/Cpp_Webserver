/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 18:19:23 by dopereir          #+#    #+#             */
/*   Updated: 2026/07/16 00:11:20 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"
#include "../http/Router.hpp"
#include "../server/Connection.hpp"

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

CgiRequestHandler::~CgiRequestHandler() {
	freeEnvp();
}

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
	if (body.size() > 0)
		res.setBody(body);
}

void	CgiRequestHandler::handleRequest(Connection& conn) {
	extractMetaVars( conn.req );
	cgiExecutor(conn); //still need to handle timeout throw 504
}

void	CgiRequestHandler::handleRequest( HttpRequest &req, HttpResponse &res ) {
	(void)req;
	(void)res;
	std::cout << "CgiRequestHandler::handleRequest virtual function called IGNORE" << std::endl;
}

/// @brief 
/// @param conn 
void	CgiRequestHandler::finalizeCgi(Connection& conn) {
	cgiResponseType	type;
	HttpResponse	newRes;
	HttpRequest		newReq;

	parseCgiHttpResponse(conn.res, conn.cgiData.outputBuffer);
	type = classifyCgiResponse(conn.res);
	switch(type) {
		case CGI_DOCUMENT:
			std::cout << "@@@@@@@@ DEBUG: CGI DOCUMENT" << std::endl;
			conn.state = WRITING;
			break;
		case CGI_LOCAL_REDIR:
			newReq = processLocalRedir(conn.res);
			newReq.setRedirectCount(conn.req.getRedirectCount() + 1);
			
			std::cout << "******* PRINT INTERNAL REQUEST ******" << std::endl;
			printRequest(newReq);

			if (conn.req.getRedirectCount() > 10)
				throw HttpException(508, "Loop Detected");

			if (newReq.getUrl().find("/cgi-bin") != std::string::npos) {
				conn.req = newReq;
				conn.res = HttpResponse();//maybe issue is here
				conn.cgiData.outputBuffer.clear();
				extractMetaVars(conn.req);
				cgiExecutor(conn);
				//need to push back fds into pollfds and cgifdToPollfd
				conn.state = RUNNING;
				std::cout << "@@@@@@@@ DEBUG: CGI LOCAL REDIR -> CGI" << std::endl;
			} else {
				Router	tmpRouter;
				tmpRouter.setConfig(_globalConfig);
				tmpRouter.setConnEnv(conn);
				tmpRouter.resolve(newReq, conn.res);
				//need to push back fds
				conn.state = WRITING;
				std::cout << "@@@@@@@@ DEBUG: CGI LOCAL REDIR -> STATIC" << std::endl;
			}
			std::cout << "@@@@@@@@ DEBUG: CGI LOCAL REDIR (END)" << std::endl;

			break;
		case CGI_CLIENT_REDIR:
			processClientRedir(conn.res);
			conn.state = WRITING;
			std::cout << "@@@@@@@@ DEBUG: CGI CLIENT REDIR" << std::endl;
			break;
		case CGI_CLIENT_DOC_REDIR:
			processClientRedirWithDocument(conn.res);
			conn.state = WRITING;
			std::cout << "@@@@@@@@ DEBUG: CGI CLIENT DOCUMENT REDIR" << std::endl;
			break;
		default:
			std::cout << "@@@@@@@@ DEBUG: NONE" << std::endl;
			throw HttpException(502, "Bad Gateway: Invalid CGI output no type match");
			break;
	}
	
	if (conn.state != RUNNING) {
		if (_envp)
			freeEnvp();
		_envp = NULL;
		conn.cgiData.pid = -1;
		conn.cgiData.inFd = -1;
		conn.cgiData.outFd = -1;
		conn.cgiData.cgiLastActivity = 0;
	}
}
