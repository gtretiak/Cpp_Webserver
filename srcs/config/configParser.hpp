/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 09:10:22 by dopereir          #+#    #+#             */
/*   Updated: 2026/04/26 22:40:51 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP
# include "globalConfig.hpp"
# include <fstream>
# include <string.h>

class	configParser {
private:
	std::vector<std::string>	_tokens;
	char*						_buffer;
	size_t						_pos;
	size_t						_length;

public:
	~configParser();

	globalConfig	parse( const std::string &filename );
	

};

std::vector<std::string>	tokenize( const char* content );

#endif