/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   globalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 18:13:28 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/21 20:28:16 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "globalConfig.hpp"
#include <iostream>
#include <sstream>

/// @brief this function intend to setup the listening ports and more necessary
/// @brief directives, from config the main process.
/// @param webserv maybe a singleton here.
void	globalConfig::initServerRoutine(Server& webserv,
			std::vector<serverConfig>& servers) {
	std::vector<serverConfig>::iterator	it = servers.begin();
	
	for (; it != servers.end(); it++) {
		for (size_t	i = 0; i < it->_listens.size(); i++) {
			std::cout << "Setting up listen[" << i << "]" << std::endl;
			webserv.setup(it->_listens[i]);
		}
	}
}

void	globalConfig::printData() const
{
	std::vector<serverConfig>::const_iterator it = servers.begin();
	
	for (; it != servers.end(); ++it)
	{
		std::cout << "Server Block:" << std::endl;
		std::cout << "  Listens:" << std::endl;
		for (size_t i = 0; i < it->_listens.size(); ++i)
		{
			std::cout << "    - " << it->_listens[i].addr << ":"
					<< it->_listens[i].port << "\ttype: "
					<< it->_listens[i].type << std::endl;
		}
		std::cout << "  Server Names:" << std::endl;
		for (size_t i = 0; i < it->_server_names.size(); ++i)
		{
			std::cout << "    - " << it->_server_names[i] << std::endl;
		}
		std::cout << "  Root: " << it->_root << std::endl;
		std::cout << "  Alias: " << it->_alias << std::endl;
		std::cout << "  Index:" << std::endl;
		for (size_t i = 0; i < it->_index.size(); ++i)
		{
			std::cout << "    - " << it->_index[i] << std::endl;
		}
		std::cout << "  Autoindex: " << (it->_autoindex ? "on" : "off") << std::endl;
		std::string clientMaxBodySize;
		if (it->_has_client_max_body_size)
		{
			std::ostringstream oss;
			oss << it->_client_max_body_size;
			clientMaxBodySize = oss.str();
		}
		else
			clientMaxBodySize = "not set";
		std::cout << "  Client Max Body Size: " << clientMaxBodySize << std::endl;

		std::string methods;
		if (it->_has_limit_except)
		{
			methods = "";
			if (it->_allowed_methods.GET)
				methods += "GET ";
			if (it->_allowed_methods.POST)
				methods += "POST ";
			if (it->_allowed_methods.DELETE)
				methods += "DELETE";
			if (methods.empty())
				methods = "not set";
		}
		else
			methods = "not set";
		std::cout << "  Allowed Methods: " << methods << std::endl;
		std::cout << "  Error Pages:" << std::endl;
		for (std::map<int, std::string>::const_iterator ep_it = it->_error_pages.begin(); ep_it != it->_error_pages.end(); ++ep_it)
		{
			std::cout << "    - " << ep_it->first << ": " << ep_it->second << std::endl;
		}
		std::cout << "  Return:" << std::endl;
		if (it->_has_return)
		{
			std::map<int, std::string>::const_iterator ret_it = it->_return.begin();

			std::cout << "    - code: " <<  ret_it->first << std::endl;
			std::cout << "    - url/text: " << ret_it->second << std::endl;
		}
		else
			std::cout << "    - not set" << std::endl;
		std::cout << "  Locations:" << std::endl;
		for (size_t i = 0; i < it->_locations.size(); ++i)
		{
			std::cout << "************ START OF LOCATION BLOCK " << i << " ************" << std::endl;
			printLocations(it->_locations[i]);
			std::cout << "************ END OF LOCATION BLOCK " << i << " ************" << std::endl;
		}
		/*std::cout << "  Directives:" << std::endl;
		for (size_t i = 0; i < it->_directives.size(); ++i)
		{
			printDirective(it->_directives[i]);
		}*/
	}
	
}

void	globalConfig::printLocations(const locationConfig& location) const
{
	std::cout << "		Locations Block:" << std::endl;
	std::cout << "			Location Path: " << location._path << std::endl;
	std::cout << "			Root: " << location._root << std::endl;
	std::cout << "			Alias: " << location._alias << std::endl;
	std::cout << "			Index:" << std::endl;
	for (size_t i = 0; i < location._index.size(); ++i)
	{
		std::cout << "				- " << location._index[i] << std::endl;
	}
	std::cout << "			Autoindex: " << (location._autoindex ? "on" : "off") << std::endl;
	std::string clientMaxBodySize;
	if (location._has_client_max_body_size)
	{
		std::ostringstream oss;
		oss << location._client_max_body_size;
		clientMaxBodySize = oss.str();
	}
	else
		clientMaxBodySize = "not set";
	std::cout << "			Client Max Body Size: " << clientMaxBodySize << std::endl;

	std::string methods;
	if (location._has_limit_except)
	{
		methods = "";
		if (location._allowed_methods.GET)
			methods += "GET ";
		if (location._allowed_methods.POST)
			methods += "POST ";
		if (location._allowed_methods.DELETE)
			methods += "DELETE";
		if (methods.empty())
			methods = "not set";
	}
	else
		methods = "not set";
	std::cout << "			Allowed Methods: " << methods << std::endl;
	std::cout << "			Error Pages:" << std::endl;
	for (std::map<int, std::string>::const_iterator ep_it = location._error_pages.begin(); ep_it != location._error_pages.end(); ++ep_it)
	{
		std::cout << "				- " << ep_it->first << ": " << ep_it->second << std::endl;
	}
	std::cout << "			Try Files:" << std::endl;
	for (size_t i = 0; i < location._try_files.size(); ++i)
	{
		std::cout << "				- " << location._try_files[i] << std::endl;
	}
	/*std::cout << "  Directives:" << std::endl;
	for (size_t i = 0; i < location._directives.size(); ++i)
	{
		printDirective(location._directives[i]);
	}*/
	std::cout << "			Return:" << std::endl;
		if (location._has_return)
		{
			std::map<int, std::string>::const_iterator ret_it = location._return.begin();

			std::cout << "				- code: " <<  ret_it->first << std::endl;
			std::cout << "				- url/text: " << ret_it->second << std::endl;
		}
		else
			std::cout << "				- not set" << std::endl;
	
	if (location._has_cgi)    {
		std::cout << "			CGI Config:" << std::endl;
		std::cout << "				Upload Store: " << location.upload_store << std::endl;
		std::cout << "				CGI Extensions:" << std::endl;
		for (std::map<std::string, std::string>::const_iterator cgi_it = location._cgi.cgi_extension.begin(); cgi_it != location._cgi.cgi_extension.end(); ++cgi_it)
		{
			std::cout << "				- " << cgi_it->first << ": " << cgi_it->second << std::endl;
		}
	}
}

void	globalConfig::printDirective(const directiveValue& directive) const
{
	std::cout << "  Directive: " << directive.name << std::endl;
	std::cout << "  Arguments:" << std::endl;
	for (size_t i = 0; i < directive.args.size(); ++i)
	{
		std::cout << "    - " << directive.args[i] << std::endl;
	}
}