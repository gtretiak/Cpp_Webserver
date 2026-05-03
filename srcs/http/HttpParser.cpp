/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:25 by gtretiak          #+#    #+#             */
/*   Updated: 2026/05/03 21:15:31 by gtretiak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "HttpParser.hpp"
#include "HttpException.hpp"
#include <string>
#include <cctype>
#include <cstdlib>
#include <vector>
#include "Connection.hpp"
#include <iostream>

HttpParser::HttpParser() {}

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
	while (buf[i] && buf[i] != ' ')
		req->setMethod(req->getMethod() + buf[i++]);
	if (req->getMethod() != "DELETE" && req->getMethod() != "GET" && req->getMethod() != "POST")
		throw HttpException(405, "Method Not Allowed");
	while (buf[i] && buf[i] == ' ')
		i++;
	if (!buf[i])
		throw HttpException(400, "Bad Request");
	while (buf[i] && buf[i] != ' ')
		req->setPath(req->getPath() + buf[i++]);
	if (req->getPath().empty())
		throw HttpException(400, "Bad Request");
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
	j = req->getPath().find("?");
	if (j != std::string::npos)
	{
		req->setQuery(req->getPath().substr(j + 1));
		req->setPath(req->getPath().substr(0, j));
	}
	while (buf[i] && buf[i] == ' ')
		i++;
	if (!buf[i])
		throw HttpException(400, "Bad Request");
	while (buf[i] && buf[i] != ' ')
		req->setVersion(req->getVersion() + buf[i++]);
	if (req->getVersion() != "HTTP/1.0" && req->getVersion() != "HTTP/1.1")
		throw HttpException(400, "Bad Request");
}

static std::string	toLower(const std::string &key) {
	std::string	res = key;
	for (size_t i = 0; i < res.size(); i++)
		res[i] = std::tolower(res[i]);
	return (res);
}

void	HttpParser::parseHeaders(std::string &buf, HttpRequest *req) {
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
		std::string	value = line.substr(column + 1);
		while (!value.empty() && value[0] == ' ') // to remove leading spaces after column
			value.erase(0, 1);
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
		if (req->getHeader("transfer-encoding") != "chunked")
			throw HttpException(501, "Not Implemented");
	}	
}

void	HttpParser::parseBody(std::string &buf, HttpRequest *req) {
	if (req->hasHeader("content-length"))
	{
		int	len = std::atoi(req->getHeader("content-length").c_str());
		if (len < 0 || (size_t)len > buf.size())
		       throw HttpException(400, "Bad Request");
		req->setBody(buf.substr(0, len));
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
			if (chunkSize == 0)
				break ;
			i = lineEnd + 2;
			if (i + chunkSize > buf.size())
				throw HttpException(400, "Bad Request");
			res += buf.substr(i, chunkSize);
			i += chunkSize + 2;
		}
		req->setBody(res);
	}
}

size_t	HttpParser::parseRequest(std::string &buf, HttpRequest *req) {
	size_t	size = 0;
	*req = HttpRequest();
	size_t	i = buf.find("\r\n");
	if (i == std::string::npos)
		throw HttpException(400, "Bad Request");
	req->setUrl(buf.substr(0, i));
	parseLine(req->getUrl(), req);
	size_t	j = buf.find("\r\n\r\n");
	if (j == std::string::npos)
		throw HttpException(400, "Bad Request");
	size = j + 4;
	std::string	header = buf.substr(i + 2, j - (i + 2));
	parseHeaders(header, req);
	if (req->hasHeader("content-length"))
	{
		int	len = std::atoi(req->getHeader("content-length").c_str());
		std::string	body = buf.substr(j + 4, len);
		parseBody(body, req);
		size += len;
	}
	else if (req->hasHeader("transfer-encoding"))
	{
		size_t	bodyEnd = buf.find("0\r\n\r\n", j + 4);
		if (bodyEnd == std::string::npos)
			throw HttpException(400, "Bad Request");
		std::string	body = buf.substr(j + 4, bodyEnd - (j + 4) + 5);
		parseBody(body, req);
		size = bodyEnd + 5;
	}
	return (size);
}

bool	HttpParser::isRequestComplete(const Connection &conn) const {
	std::string const	&buf = conn.readBuffer;
	size_t	headerEnd = buf.find("\r\n\r\n"); // header completeness
	if (headerEnd == std::string::npos)
		return (false);
	std::string	LnH = buf.substr(0, headerEnd);
	size_t	firstLineEnd = LnH.find("\r\n");
	if (firstLineEnd == std::string::npos)
		return (false);
	std::string	firstL = LnH.substr(0, firstLineEnd);
	if (firstL.find("GET ") == 0 || firstL.find("DELETE ") == 0)
		return (true);
	std::string	lowLnH = toLower(LnH);
	size_t	clPos = lowLnH.find("content-length:");
	if (clPos != std::string::npos) // content-length presented
	{
		size_t	start = clPos + 15;
		while (start < LnH.size() && LnH[start] == ' ')
			start++;
		size_t	end = start;
		while (end < LnH.size() && std::isdigit(LnH[end]))
			end++;
		int	len = std::atoi(LnH.substr(start, end - start).c_str());
		return (buf.size() >= headerEnd + 4 + len);
	}
	clPos = lowLnH.find("transfer-encoding:");
	if (clPos != std::string::npos) // transfer-encoding presented
	{
		size_t	start = clPos + 18;
		while (start < LnH.size() && LnH[start] == ' ')
			start++;
		return (buf.find("0\r\n\r\n", headerEnd + 4) != std::string::npos);
	}
	return (false);
}

HttpParser::~HttpParser() {}
