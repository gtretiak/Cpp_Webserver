/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   locationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 23:24:56 by dopereir          #+#    #+#             */
/*   Updated: 2026/05/29 00:23:41 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "locationConfig.hpp"
# include <cctype>

locationConfig::locationConfig()
	: _path(),
	_root(),
	_alias(),
	_index(),
	_autoindex(false),
	_has_autoindex(false),
	_client_max_body_size(0),
	_has_client_max_body_size(false),
	_allowed_methods(),
	_has_limit_except(false),
	_error_pages(),
	_try_files(),
	_directives(),
	_redirect(),
	_redirect_code(0),
	_cgi(),
	_has_cgi(false) {}

locationConfig::locationConfig(const locationConfig& other)
	: _path(other._path),
	_root(other._root),
	_alias(other._alias),
	_index(other._index),
	_autoindex(other._autoindex),
	_has_autoindex(other._has_autoindex),
	_client_max_body_size(other._client_max_body_size),
	_has_client_max_body_size(other._has_client_max_body_size),
	_allowed_methods(other._allowed_methods),
	_has_limit_except(other._has_limit_except),
	_error_pages(other._error_pages),
	_try_files(other._try_files),
	_directives(other._directives),
	_redirect(other._redirect),
	_redirect_code(other._redirect_code),
	_cgi(other._cgi),
	_has_cgi(other._has_cgi) {
}

locationConfig& locationConfig::operator=(const locationConfig& other) {
	if (this != &other) {
		_path = other._path;
		_root = other._root;
		_alias = other._alias;
		_index = other._index;
		_autoindex = other._autoindex;
		_has_autoindex = other._has_autoindex;
		_client_max_body_size = other._client_max_body_size;
		_has_client_max_body_size = other._has_client_max_body_size;
		_allowed_methods = other._allowed_methods;
		_has_limit_except = other._has_limit_except;
		_error_pages = other._error_pages;
		_try_files = other._try_files;
		_directives = other._directives;
		_redirect = other._redirect;
		_redirect_code = other._redirect_code;
		_cgi = other._cgi;
		_has_cgi = other._has_cgi;
	}
	return *this;
}

locationConfig::~locationConfig() {
}

bool	isValidCgiExtention (const std::string& value) {
	if (value.size() < 2 || value[0] != '.') {
		return false;
	}
	for (size_t i = 1; i < value.size(); ++i) {
		if (!std::isalnum(static_cast<unsigned char>(value[i]))) {
			return false;
		}
	}
	return true;
}