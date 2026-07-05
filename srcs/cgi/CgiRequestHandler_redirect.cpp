/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_redirect.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 14:17:59 by dopereir          #+#    #+#             */
/*   Updated: 2026/07/02 22:52:03 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"


CgiRequestHandler::cgiResponseType	CgiRequestHandler::classifyCgiResponse( HttpResponse& res ) {
	std::map<std::string, std::string>	headers = res.getHeaders();
	std::string		query;
	cgiResponseType	type;
	std::string		loc;

	type = NONE;
	if (res.hasHeader("location")) {
		loc = res.getHeader("location");

		if (loc[0] == '/' && headers.size() == 1 && res.getBody().empty()) {
			query = getQueryFromURI(loc);

			if (!query.empty())
				res.setHeader("query-string", query);
			type = CGI_LOCAL_REDIR;
		}
		else if ((loc.find("http://") == 0 || loc.find("https://") == 0)
			&& headers.size() == 1 && res.getBody().empty()) {
			query = getQueryFromURI(loc);

			if (!query.empty())
				res.setHeader("query-string", query);
			type = CGI_CLIENT_REDIR;
		}
		else if ((loc.find("http://") == 0 || loc.find("https://") == 0)
			&& res.hasHeader("content-type") && res.hasHeader("status")
			&& (res.getStatusCode() >= 300 && res.getStatusCode() <= 308)
			&& headers.size() <= 3 && !res.getBody().empty()) {
			type = CGI_CLIENT_DOC_REDIR;
		}
		else
			throw HttpException(502, "Bad Gateway: Invalid CGI output no type match");
	}
	else {
		if (!res.hasHeader("content-type"))
			throw HttpException(502, "Bad Gateway: Invalid CGI output: no content-type");
		type = CGI_DOCUMENT;
	}
	return type;
}


/// @brief METHOD always become GET,
///
/// @param res 
HttpRequest	CgiRequestHandler::processLocalRedir( HttpResponse& res) {
	HttpRequest	newReq;
	std::string	target;
	std::string	path;
	std::string	query;
	
	target = res.getHeader("location");
	path = getPathFromURI(target);
	query = getQueryFromURI(target);

	newReq.setUrl(target);
	newReq.setPath(path);
	newReq.setQuery(query);
	newReq.setMethod("GET");
	newReq.setVersion("HTTP/1.1");

	return newReq;
	//build new request
	//request line = METHOD + LOCATION (URL) + VERSION
	//send back to router
	//let the router decide, static file? cgi? error? ...
	
	//basically will loop back to the router until is a static or cgi plain
	// document response
	// must avoid recursion so we need to detect multiple local redirects.
}

void	CgiRequestHandler::processClientRedir( HttpResponse& res ) {
	res.setVersion("HTTP/1.1");
	res.setHeader("status", "302");
	res.setHeader("content-length", "0");
	res.setBody("");
}

void	CgiRequestHandler::processClientRedirWithDocument( HttpResponse& res ) {
	printResponse(res);
}