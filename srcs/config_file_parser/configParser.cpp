/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 09:18:39 by dopereir          #+#    #+#             */
/*   Updated: 2026/04/23 11:50:00 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "configParser.hpp"

configParser::~configParser() {
	//delete[]	_buffer; //or call it in the same scope
};

std::vector<std::string>	tokenize( const char* content ) {
	std::vector<std::string>	tokens;
	std::string					curr;
	char						c;
	int							len;

	len = strlen(content);
	for (size_t i = 0; i < len; i++) {
		c = content[i];
		if (c == '#') {
			while (i < len && content[i] != '\n')
				i++;
		}
		else if (c == '{' || c == '}' || c == ';') {
			if (!curr.empty()) {
				tokens.push_back(curr);
				curr.clear();
			}
			tokens.push_back(std::string(1, c));
		}
		else if (isspace(c)) {
			if (!curr.empty()) {
				tokens.push_back(curr);
				curr.clear();
			}
		}
		else {
			curr += c;
		}
	}
	return tokens;
};

globalConfig	configParser::parse( const std::string &filename ) {
	std::fstream	fs;
	globalConfig	config;


	fs.open(filename.c_str(), std::fstream::in);
	if (fs)
	{
		fs.seekg(0, fs.end);
		_length = fs.tellg();//returns a streampos/fpos type which can be converted to/from integer types
		fs.seekg(0, fs.beg);

		_buffer = new char [_length];
		fs.read(_buffer, _length);

		_tokens = tokenize(_buffer);

		/* for (std::vector<std::string>::iterator it = _tokens.begin(); it != _tokens.end(); ++it) {
			std::cout << "'" << *it << "'" << "\n";
		} */

		delete[] _buffer;
		fs.close();
	}
	return config;
};