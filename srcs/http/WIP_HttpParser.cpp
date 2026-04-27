/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WIP_HttpParser.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:25 by gtretiak          #+#    #+#             */
/*   Updated: 2026/04/27 20:44:51 by gtretiak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include "HttpException.hpp"

static std::string	normalize(std::string &url) {
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
		token = url.substr(start, end - i);
		if (token == "" || token == ".")
			;
		else if (token == "..")
		{
			if (stack.empty())
				return (NULL);
			stack.pop_back();
		}
		else
			stack.push_back(token);
		start = end + 1;
	}
	for (int i = 0; i < stack.size(); i++)
	{
		res += stack[i];
		if (i != stack.size() - 1)
			res += "/";
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

static std::string	decode(std::string &url) {
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
	}
	res += url[i];
	i++;
	return (res);
}

void	HttpParser::parseLine(std::string &buf, HttpRequest *req) {
	size_t	i = 0;
	size_t	j;
	while (buf[i] && buf[i] != ' ')
		req->method += buf[i++];
	if (req->method != "DELETE" && req->method != "GET" && req->method != "POST")
		throw HttpException(405, "Method Not Allowed");
	while (buf[i] && buf[i] == ' ')
		i++;
	if (!buf[i])
		throw HttpException(400, "Bad Request");
	while (buf[i] && buf[i] != ' ')
		req->path += buf[i++];
	if (req->path.empty())
		throw HttpException(400, "Bad Request");
	if (req.path.find("http://") != std::string::npos)
	{
		size_t	pos = req.path.find("/", 7);
		if (pos == std::string::npos)
			req.path = "/";
		else
			req.path = req.path.substr(pos);
	}
	req.path = decode(req.path);
	req.path = normalize(req.path);
	if (!req.path)
		throw HttpException(403, "Forbidden");
	j = req->path.find("?");
	if (j != std::string::npos)
	{
		req->query = req->path.substr(j + 1);
		req->path = req->path.substr(0, j);
	}
	while (buf[i] && buf[i] == ' ')
		i++;
	if (!buf[i])
		throw HttpException(400, "Bad Request");
	while (buf[i] && buf[i] != ' ')
		req->version += buf[i++];
	if (req->version.find("HTTP/1.0") == std::string::npos && req->version.find("HTTP/1.1") == std::string::npos && buf[i - 2] != '.')
		throw HttpException(400, "Bad Request");
}

void	HttpParser::parseHeaders(std::string &buf, HttpRequest *req) {
	if (req->version == "HTTP/1.1")
	{
		if (req->headers.find("host") == req->headers.end())
			throw HttpException(400, "Bad Request");
	}
	if (req->headers.find("transfer-encoding") == req->headers.end())
		throw HttpException(400, "Bad Request");
}

void	HttpParser::parseBody(std::string &buf, HttpRequest *req) {
	if (req.headers["content-length"])
	{
		for (size_t i = 0; i < buf.length() && i < atoi(req.headers["content-length"]); i++)
			req->body += buf[i];
	}
	else
	{
		for (size_t i = 0; i < buf.length(); i++)
		{
			size_t	pos = buf.substr(i).find("\r\n");
			req->body += buf.substr(i, pos - 1);
			i = pos + 2;
		}
	}
}

void	HttpParser::parseRequest(std::string &buf, HttpRequest *req) {
	size_t	i = buf.find("\r\n");
	if (i == std::string::npos)
		throw HttpException(400, "Bad Request");
	std::string	line = buf.substr(0, i - 1);
	parseLine(line, req);
	size_t	j = buf.find("\r\n\r\n");
	if (j == std::string::npos)
		throw HttpException(400, "Bad Request");
	std::string	header = buf.substr(i + 2, j - 1);
	parseHeaders(header, req);
	if (req->method == "POST")
	{
		std::string	body = buf.substr(j + 4);
		parseBody(body, req);
	}
}

bool	HttpParser::isRequestComplete(const Connection &conn) const {
	std::string	&buf = conn.readBuffer;
	size_t	headerEnd = buf.find("\r\n\r\n"); // header completeness
	if (headerEnd == std::string::npos)
		return (false);
	std::string	LnH = buf.substr(0, headerEnd);
	if (LnH.find("GET") != std::string::npos || LnH.find("DELETE") != std::string::npos)
		return (true);
	size_t	CL = LnH.find("ength");
	bool	isChunked = false;
	if (CL != std::string::npos)
	{
		if (LnH.find("hunked") != std::string::npos)
			throw HttpException(400, "Bad Request");
	}
	else
	{
		isChunked = true;
		CL = LnH.find("hunked");
		if (CL == std::string::npos)
			throw HttpException(400, "Bad Request");
	}
	int	bodySize = 0;
	if (!isChunked)
	{
		int	i = 5;
		if (LnH[CL + i++] != ':')
			throw HttpException(400, "Bad Request");
		while (LnH[CL + i] == ' ')
			i++;
		while (LnH[CL + i] >= '0' && LnH[CL + i] <= '9')
		{
			bodySize *= 10;
			bodySize += LnH[CL + i] - '0';
			i++;
		}
		if (LnH[CL + i] != '\r')
			throw HttpException(400, "Bad Request");
		conn.request.headers["content-length"] = bodySize;
		if (buf.substr(headerEnd + 4).size() >= bodySize)
			return (true);
		return (false);
	}
	if (isChunked)
	{
		conn.request.headers["transfer-encoding"] = "chunked";
		if (buf.substr(headerEnd + 4).find("0\r\n\r\n") != std::string::npos)
			return (true);
		return (false);
	}
	return (true);
}
