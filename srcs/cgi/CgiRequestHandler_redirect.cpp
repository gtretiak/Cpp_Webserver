/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_redirect.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 14:17:59 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/11 12:04:11 by dopereir         ###   ########.fr       */
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