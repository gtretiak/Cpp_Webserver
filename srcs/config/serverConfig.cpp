/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:27:24 by dopereir          #+#    #+#             */
/*   Updated: 2026/07/14 11:25:59 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "serverConfig.hpp"

serverConfig::serverConfig()
	: _listens(),
	_server_names(),
	_root(),
	_alias(),
	_index(),
	_autoindex(false),
	_client_max_body_size(0),
	_has_client_max_body_size(false),
	_allowed_methods(),
	_has_limit_except(false),
	_error_pages(),
	_locations(),
	_directives(),
	_return(),
	_has_return(false),
	_upload_store() {}

serverConfig::serverConfig(const serverConfig& other)
	: _listens(other._listens),
	_server_names(other._server_names),
	_root(other._root),
	_alias(other._alias),
	_index(other._index),
	_autoindex(other._autoindex),
	_client_max_body_size(other._client_max_body_size),
	_has_client_max_body_size(other._has_client_max_body_size),
	_allowed_methods(other._allowed_methods),
	_has_limit_except(other._has_limit_except),
	_error_pages(other._error_pages),
	_locations(other._locations),
	_directives(other._directives),
	_return(other._return),
	_has_return(other._has_return),
	_upload_store(other._upload_store) {}

serverConfig& serverConfig::operator=(const serverConfig& other) {
	if (this != &other) {
		_listens = other._listens;
		_server_names = other._server_names;
		_root = other._root;
		_alias = other._alias;
		_index = other._index;
		_autoindex = other._autoindex;
		_client_max_body_size = other._client_max_body_size;
		_has_client_max_body_size = other._has_client_max_body_size;
		_allowed_methods = other._allowed_methods;
		_has_limit_except = other._has_limit_except;
		_error_pages = other._error_pages;
		_locations = other._locations;
		_directives = other._directives;
		_return = other._return;
		_has_return = other._has_return;
		_upload_store = other._upload_store;
	}
	return *this;
}

serverConfig::~serverConfig() {}