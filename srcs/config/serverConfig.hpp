/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 23:20:08 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/05 17:13:57 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
# include "locationConfig.hpp"

class	serverConfig {
public:
	std::vector<Listen>			_listens;
	std::vector<std::string>	_server_names;
	std::string					_root;
	std::string					_alias; //not mandatory, i guess
	std::vector<std::string>	_index;
	bool						_autoindex;
	bool						_has_autoindex;
	size_t						_client_max_body_size;
	bool						_has_client_max_body_size;
	limitExcept					_allowed_methods;
	bool						_has_limit_except;
	std::map<int, std::string>	_error_pages;
	std::vector<locationConfig>	_locations;
	std::vector<directiveValue>	_directives;
	std::map<int, std::string>	_return;
	bool						_has_return;

	serverConfig();
	serverConfig(const serverConfig& other);
	serverConfig &operator=(const serverConfig& other);
	~serverConfig();
};

#endif
