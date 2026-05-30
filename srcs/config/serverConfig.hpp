/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 23:20:08 by dopereir          #+#    #+#             */
/*   Updated: 2026/04/26 22:40:00 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCOFIG_HPP
# include "locationConfig.hpp"
# include <iostream>
# include <vector>
# include <map>

# define WSV_ERROR 1		//there is a error
# define WSV_OK 2			//the token terminated by ";" was found
# define WSV_BLOCK_START 3	//the token terminated by "{" was found
# define WSV_BLOCK_END 4	//the token terminated by "}" was found
# define WSV_FILE_DONE 5	//the config file is done

struct Listen {
	enum listenTypeImpl {
		PORT,
		IP_PORT,
		HOST_PORT,
		UNIX_SOCK
	};//BE CAREFUL OF IMPLICIT TYPE CONVERSION
	typedef Listen::listenTypeImpl	listenType;

	listenType	type;
	std::string	addr;
	int			port;
};

struct	limitExcept {
	bool	GET;
	bool	POST;
	bool	DELETE;

	limitExcept() : GET(true), POST(true), DELETE(true) {}
};

class	serverConfig {
private:
	//variable to store the listen directive
	Listen						_varListen;
	//error_pages
	std::map<int, std::string>	_error_pages;
	//client_max_body_size
	size_t						_client_max_body_size;//default 1m
	//limit_except
	limitExcept					_allowedMethods;
	//root/alias
	std::string					_root;
	std::string					_alias;
	//autoindex
	bool						_autoindex;//default on
	//index
	std::string					_index;
	

	//locationBlock class INHERITS from serverConfig
	std::vector<locationConfig>	locations;
public:
	serverConfig();
	serverConfig( const serverConfig& other );
	serverConfig &operator=( const serverConfig& other );
	~serverConfig();

	serverConfig	parseServerBlock();
};

//USE RECURSCIVE DESCENT PARSER
/*
 Config
 └── ServerBlock[]
      ├── directives (listen, server_name, ...)
      └── LocationBlock[]
            └── directives (root, index, methods, ...) */


#endif