/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WIP_HttpParser.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:25 by gtretiak          #+#    #+#             */
/*   Updated: 2026/04/27 11:59:51 by gtretiak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

HttpRequest	&HttpParser::parse(std::string &buf) {
	HttpRequest	req;
	req.method = ...;
	req.path = ...;
	req.headers["host"] = ...;
	req.body = ...;
	return (req);
}

bool	HttpParser::isRequestComplete(const Connection &conn) const {
	std::string	&buf = conn.readBuffer;
	size_t	headerEnd = buf.find("\r\n\r\n"); // header completeness
	if (headerEnd == std::string::npos)
		return (false);
	std::string	header = buf.substr(0, headerEnd);
	if (header.find("GET") || header.find("DELETE"))
		return (true);
	size_t	contType = header.find("Content-Length");
	bool	isChunked = false;
	if (contType)
	{
		if (header.find("Transfer-Encoding: chunked"))
			return 400 Bad Request?
	}
	else
	{
		isChunked = true;
		contType = header.find("Transfer-Encoding: chunked");
		if (contentType == std::string::npos)
			return 400 Bad Request?
	}
	int	bodySize = 0;
	if (header[contType] == 'C')
	{
		for (int i = 15; header[contType + i] >= '0' && header[contType + i] <= '9'; i++)
		{
			bodySize *= 10;
			bodySize += header[contType + i] - '0';
		}
	}
	if (isChunked || header[contType] == 'T')
		looking for '0\r\n\r\n';
	else
		keeping track of received body (should be equal to bodySize);
	
	if GET - check path and normalise it
		received body size should equal to Content-length;
	if (no host)
		return 400 Bad Request
	else
		?;
	return (true);
}
