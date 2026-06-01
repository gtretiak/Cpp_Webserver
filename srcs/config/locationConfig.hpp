/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   locationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 11:54:28 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/01 09:44:58 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <cstddef>
# include <map>
# include <string>
# include <vector>

struct Listen {
	enum listenTypeImpl {
		PORT,
		IP_PORT,
		HOST_PORT,
		UNIX_SOCK
	};
	typedef Listen::listenTypeImpl	listenType;

	listenType	type;
	std::string	addr;
	int			port;

	Listen() : type(PORT), addr(), port(0) {}
};

struct limitExcept {
	bool	GET;
	bool	POST;
	bool	DELETE;

	limitExcept() : GET(true), POST(true), DELETE(true) {}
};

struct directiveValue {
	std::string					name;
	std::vector<std::string>	args;
};

struct cgiConfig {
	std::map<std::string, std::string>	cgi_extension;
	std::string							upload_store;
};

class	locationConfig {
public:
	std::string					_path;
	std::string					_root;
	std::string					_alias;
	std::vector<std::string>	_index;
	bool						_autoindex;
	bool						_has_autoindex;
	size_t						_client_max_body_size;
	bool						_has_client_max_body_size;
	limitExcept					_allowed_methods;
	bool						_has_limit_except;
	std::map<int, std::string>	_error_pages;
	std::vector<std::string>	_try_files;
	std::vector<directiveValue>	_directives;
	std::string					_redirect;
	int							_redirect_code;
	cgiConfig					_cgi;
	bool						_has_cgi;

	locationConfig();
	locationConfig(const locationConfig& other);
	locationConfig& operator=(const locationConfig& other);
	~locationConfig();
};

bool	isValidCgiExtention(const std::string& value);

#endif
