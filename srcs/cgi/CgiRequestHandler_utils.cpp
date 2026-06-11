/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_utils.cpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 17:45:18 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/11 12:03:58 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"

void	CgiRequestHandler::freeEnvp( ) {
	if (!_envp)
		return ;
	for (int i = 0; _envp[i]; ++i) {
		free(_envp[i]);
	}
	_envp = NULL;
	return ;
}

void	CgiRequestHandler::printEnvp( ) {
	if (!_envp)
		return ;
	for (int i = 0; _envp[i]; ++i) {
		std::cout << "_envp[" << i << "]:\t\t" << _envp[i] << std::endl;
	}
}

void	CgiRequestHandler::printMetaVars( ) {
	std::map<std::string, std::string>::iterator	it = _meta_vars.begin();

	for (; it != _meta_vars.end(); ++it) {
		std::cout << "Key: " << it->first << "\tValue: " << it->second << std::endl;
	}
}

int	CgiRequestHandler::getClientMaxBodySize( ) {
	int	size = CLIENT_MAX_BODY_SIZE;

	if (_locSetting->_has_client_max_body_size) {
		size = _locSetting->_client_max_body_size;
	}
	else if (_serverSetting->_has_client_max_body_size) {
		size = _serverSetting->_client_max_body_size;
	}
	return size;
}

void	printRequest( HttpRequest &req ) {
	const std::map<std::string, std::string>	head = req.getHeaders();

	std::map<std::string, std::string>::const_iterator	it;

	it = head.begin();
	for (; it != head.end(); ++it) {
		std::cout << "Key: " << it->first
			<< "\t\tValue: " << it->second << std::endl;
	}

	std::cout << "Method: " << req.getMethod() << std::endl;
	std::cout << "URL: " << req.getUrl() << std::endl;
	std::cout << "Path: " << req.getPath() << std::endl;
	std::cout << "Query: " << req.getQuery() << std::endl;
	std::cout << "Version: " << req.getVersion() << std::endl;
	std::cout << "Body: " << req.getBody() << std::endl;

}

void	printResponse( HttpResponse& res ) {
	std::map<std::string, std::string>::const_iterator	it;
	const std::map<std::string, std::string>			head = res.getHeaders();

	it = head.begin();
	for (; it != head.end(); ++it) {
		std::cout << "Key: " << it->first
			<< "\t\tValue: " << it->second << std::endl;
	}
	std::cout << "Version: " << res.getVersion() << std::endl;
	std::cout << "Status code: " << res.getStatusCode() << std::endl;
	std::cout << "Body: " << res.getBody() << std::endl;
}

void	buildRequest(HttpRequest& req) {
	req.setMethod("POST");
	//req.setUrl("/cgi-bin/py_script.py/api/v1/list?limit=10");
	req.setUrl("/cgi-bin/local_redirect_script.py");
	req.setPath("/cgi-bin/local_redirect_script.py");
	//req.setPath("/cgi-bin/py_script.py");
	req.setQuery("foo=bar&mode=demo");
	req.setVersion("HTTP/1.1");
	req.setHeader("host", "localhost");
	req.setHeader("user-agent", "cgi-demo/1.0");
	req.setHeader("content-type", "application/x-www-form-urlencoded");
	req.setBody("name=webserv&lang=c++98");
	{
		std::ostringstream	contentLength;
		contentLength << req.getBody().size();
		req.setHeader("content-length", contentLength.str());
	}
	req.setHeader("connection", "keep-alive");
}
