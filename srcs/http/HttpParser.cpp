/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:25 by gtretiak          #+#    #+#             */
/*   Updated: 2026/08/27 10:39:33 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "HttpParser.hpp"
#include "HttpUtils.hpp"
#include "HttpException.hpp"
#include <string>
#include <limits>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <errno.h>
#include "../server/Connection.hpp"

static const size_t	MAX_HEADER_SIZE = 8192;
//static const size_t	MAX_BODY_SIZE = 8192;//to be fetch from config; to be removed TODO

HttpParser::HttpParser() : _maxBodySize(8192) {}

void HttpParser::setMaxBodySize(size_t maxBodySize)
{
	_maxBodySize = maxBodySize;
}

static std::string	normalize(const std::string &url) {
	std::string	res = "/";
	std::vector<std::string>	stack;
	std::string	token;
	size_t	len = url.length();
	size_t	start = 0;
	while (start <= len)
	{
		size_t	end = start;
		while (end < len && url[end] != '/')
			end++;
		token = url.substr(start, end - start);
		if (token == "" || token == ".")
			;
		else if (token == "..")
		{
			if (stack.empty())
				throw HttpException(403, "Forbidden");
			stack.pop_back();
		}
		else
			stack.push_back(token);
		start = end + 1;
	}
	size_t	i = 0;
	while (i < stack.size())
	{
		res += stack[i];
		if (i != stack.size() - 1)
			res += "/";
		i++;
	}
	return (res);
}

static int	hexToInt(char c) {
	if (c >= '0' && c <= '9')
		return (c - '0');
	else if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	else if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	return (-1);
}

static std::string	decode(const std::string &url) {
	std::string	res;
	size_t	i = 0;
	while (i < url.length())
	{
		if (url[i] == '%' && i + 2 < url.length())
		{
			int	higher = hexToInt(url[i + 1]);
			int	lower = hexToInt(url[i + 2]);
			if (higher >= 0 && lower >= 0)
			{
				res += static_cast<char>(higher * 16 + lower);
				i += 3;
				continue ;
			}
		}
		res += url[i];
		i++;
	}
	return (res);
}

void	HttpParser::parseLine(const std::string &buf, HttpRequest *req) {
	size_t	i = 0;
	size_t	j;
	while (i < buf.size() && buf[i] != ' ')
		req->setMethod(req->getMethod() + buf[i++]);
	if (req->getMethod() != "DELETE" && req->getMethod() != "GET" && req->getMethod() != "POST")
		throw HttpException(405,
			"Method Not Allowed: " + req->getMethod() + " is not supported");
	while (i < buf.size() && buf[i] == ' ')
		i++;
	if (i >= buf.size())
		throw HttpException(400, "Bad Request");
	while (i < buf.size() && buf[i] != ' ')
		req->setUrl(req->getUrl() + buf[i++]);
	if (req->getUrl().empty())
		throw HttpException(400, "Bad Request");
	j = req->getUrl().find("?");
	if (j != std::string::npos)
	{
		req->setQuery(req->getUrl().substr(j + 1));
		req->setPath(req->getUrl().substr(0, j));
	}
	else
		req->setPath(req->getUrl());
	if (req->getPath().find("http://") == 0)
	{
		size_t	pos = req->getPath().find("/", 7);
		if (pos == std::string::npos)
			req->setPath("/");
		else
			req->setPath(req->getPath().substr(pos));
	}
	req->setPath(decode(req->getPath()));
	req->setPath(normalize(req->getPath()));
	while (i < buf.size() && buf[i] == ' ')
		i++;
	if (i >= buf.size())
		throw HttpException(400, "Bad Request");
	while (i < buf.size() && buf[i] != ' ')
		req->setVersion(req->getVersion() + buf[i++]);
	if (req->getVersion() == "HTTP/1.0")
		req->setHeader("connection", "close");
	else if (req->getVersion() == "HTTP/1.1")
		req->setHeader("connection", "keep-alive");
	else 
		throw HttpException(505, "HTTP Version Not Supported");
}

void	HttpParser::parseHeaders(std::string &buf, HttpRequest *req) {
	if (buf.size() > MAX_HEADER_SIZE)
		throw HttpException(413, "Payload Too Large");
	size_t	start = 0;
	while (start < buf.size())
	{
		size_t	end = buf.find("\r\n", start);
		if (end == std::string::npos)
			end = buf.size();
		std::string	line = buf.substr(start, end - start);
		if (line.empty())
			break ;
		size_t	column = line.find(":");
		if (column == std::string::npos)
			throw HttpException(400, "Bad Request");
		std::string	key = toLower(line.substr(0, column));
		size_t	valueFirst = column + 1;
		while (valueFirst < line.size() && line[valueFirst] == ' ') // to remove leading spaces after column
			valueFirst++;
		std::string	value = line.substr(valueFirst);
		while (!value.empty() && value[value.size() - 1] == ' ') // to remove trailing spaces
			value.erase(value.size() - 1, 1);
		req->setHeader(key, value);
		start = end + 2;
	}
	if (req->getVersion() == "HTTP/1.1" && !req->hasHeader("host"))
		throw HttpException(400, "Bad Request");
	bool	hasCL = req->hasHeader("content-length");
	bool	hasTE = req->hasHeader("transfer-encoding");
	if (hasCL && hasTE)
		throw HttpException(400, "Bad Request");
	if (hasCL)
	{
		const std::string	&len = req->getHeader("content-length");
		if (len.empty())
			throw HttpException(400, "Bad Request");
		for (size_t i = 0; i < len.size(); i++)
		{
			if (!std::isdigit(len[i]))
				throw HttpException(400, "Bad Request");
		}
	}
	if (hasTE)
	{
		if (toLower(req->getHeader("transfer-encoding")) != "chunked")
			throw HttpException(501, "Not Implemented");
	}	
}

void	HttpParser::parseBody(std::string &buf, HttpRequest *req) {
	if (req->hasHeader("content-length"))
	{
		char	*end;
		errno = 0;
		unsigned long	len = std::strtoul(req->getHeader("content-length").c_str(), &end, 10);
		if (errno != 0 || *end != '\0')
			throw HttpException(400, "Invalid Content-Length: non-digits presented");
		if (len == std::numeric_limits<unsigned long>::max())
			throw HttpException(400, "Invalid Content-Length: negative value");
		if (static_cast<size_t>(len) > _maxBodySize)
			throw HttpException(413, "Payload Too Large");
		if (static_cast<size_t>(len) < buf.size())
			throw HttpException(400, "Extra Data After Body");
		req->adoptBody(buf);
	}
	else // chunked
	{
		std::string	res;
		size_t	i = 0;
		while (true)
		{
			size_t	lineEnd = buf.find("\r\n", i);
			if (lineEnd == std::string::npos)
				throw HttpException(400, "Bad Request");
			int	chunkSize = std::strtol(buf.substr(i, lineEnd - i).c_str(), NULL, 16);
			// 16 means hexadecimal base
			if (chunkSize == 0)
				break ;
			i = lineEnd + 2;
			if (i + chunkSize > buf.size())
				throw HttpException(400, "Chunk Size Mismatch");
			if (buf.substr(i + chunkSize, 2) != "\r\n")
				throw HttpException(400, "Bad Request");
			res += buf.substr(i, chunkSize);
			i += chunkSize + 2;
		}
		req->adoptBody(res);
	}
}

size_t	HttpParser::parseRequest(std::string &buf, HttpRequest *req) {
	std::string	header;
	size_t		size;
	size_t		i;
	size_t		j;

	size = 0;
	*req = HttpRequest();
	i = buf.find("\r\n");
	if (i == std::string::npos)
		throw HttpException(400, "Bad Request");
	parseLine(buf.substr(0, i), req);
	j = buf.find("\r\n\r\n");
	if (j == std::string::npos)
		throw HttpException(400, "Bad Request");
	const size_t	headerEndLen = 4;
	const size_t	crlfLen = 2;
	size = j + headerEndLen;
	header = buf.substr(i + crlfLen, j - (i + crlfLen));
	parseHeaders(header, req);
	if (req->hasHeader("content-length"))
	{
		int	len = std::atoi(req->getHeader("content-length").c_str());
		std::string	body = buf.substr(j + headerEndLen, len);
		parseBody(body, req);
		size += len;
	}
	else if (req->hasHeader("transfer-encoding"))
	{
		const size_t	lastChunkLen = 5;
		size_t	bodyEnd = buf.find("0\r\n\r\n", j + headerEndLen);
		if (bodyEnd == std::string::npos)
			throw HttpException(400, "Bad Request");
		std::string	body = buf.substr(j + headerEndLen, bodyEnd - (j + headerEndLen) + lastChunkLen);
		if (bodyEnd + lastChunkLen < buf.size()) 
			throw HttpException(400, "Extra data after terminator");
		parseBody(body, req);
		size = bodyEnd + lastChunkLen;
	}
	return (size);
}

bool	HttpParser::isRequestComplete(const std::string &buf) const {
	size_t	headerEnd = buf.find("\r\n\r\n"); // header completeness
	if (headerEnd == std::string::npos)
		return (false);
	std::string	LnH = buf.substr(0, headerEnd);
	size_t	firstLineEnd = LnH.find("\r\n");
	if (firstLineEnd == std::string::npos)
		return (false);
	std::string	lowLnH = toLower(LnH);
	size_t	clPos = lowLnH.find("content-length:");
	const size_t	clStrLength = 15;
	if (clPos != std::string::npos) // content-length presented
	{
		size_t	start = clPos + clStrLength;
		while (start < LnH.size() && LnH[start] == ' ')
			start++;
		size_t	end = start;
		while (end < LnH.size() && std::isdigit(LnH[end]))
			end++;
		int	len = std::atoi(LnH.substr(start, end - start).c_str());
		return (buf.size() >= headerEnd + 4 + len);
	}
	size_t	tePos = lowLnH.find("transfer-encoding:");
	if (tePos != std::string::npos) // transfer-encoding presented
		return (buf.find("0\r\n\r\n", headerEnd + 4) != std::string::npos);
	return (true);
}

HttpParser::~HttpParser() {}
