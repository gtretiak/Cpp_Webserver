/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   locationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 11:54:28 by dopereir          #+#    #+#             */
/*   Updated: 2026/04/26 22:37:52 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP
# include <iostream>

struct	limitExcept {
	bool	GET;
	bool	POST;
	bool	DELETE;

	limitExcept() : GET(true), POST(true), DELETE(true) {}
};

// about location block behavior
//https://www.youtube.com/watch?v=3q2xxMc7XEo

class	locationConfig {
	//directives:
	/*
	1. root/alias
	2. index
	3. autoindex
	4. return
	5. limit except
	6. client_max_body_size
	7- upload/store path
	*/
	size_t			_client_max_body_size;
	limitExcept		_allowedMethods;
	std::string		_root;
	std::string		_alias;
	bool			_autoindex;
	std::string		_index;
};

#endif