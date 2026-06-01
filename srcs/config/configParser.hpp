/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 09:10:22 by dopereir          #+#    #+#             */
/*   Updated: 2026/05/29 00:21:24 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "globalConfig.hpp"
# include <stdexcept>

class	configParser {
private:
	struct Token {
		std::string	value;
		size_t		line;
	};

	std::vector<Token>	_tokens;
	size_t				_pos;

	Token				peek() const;
	const Token&		current() const;
	bool				eof() const;
	bool				isSymbol(const std::string& value) const;
	Token				consume();
	void				expect(const std::string& value);
	std::vector<Token>	tokenize(const std::string& content) const;
	globalConfig		parseConfig();
	serverConfig		parseServerBlock();
	locationConfig		parseLocationBlock();
	std::vector<std::string>	collectArguments();
	void				applyServerDirective(serverConfig& server,
							const std::string& name,
							const std::vector<std::string>& args,
							size_t line);
	void				applyLocationDirective(locationConfig& location,
							const std::string& name,
							const std::vector<std::string>& args,
							size_t line);
	Listen				parseListen(const std::string& value, size_t line) const;
	size_t				parseSize(const std::string& value, size_t line) const;
	std::string			formatError(size_t line, const std::string& message) const;
	bool				isNumber(const std::string& value) const;
	bool				isIPv4(const std::string& value) const;
	std::string			join(const std::vector<std::string>& values) const;
	void				appendAllowedMethod(limitExcept& methods, const std::string& value);

public:
	configParser();
	~configParser();

	class parseError : public std::runtime_error {
	public:
		explicit parseError(const std::string& message) : std::runtime_error(message) {}
	};

	globalConfig	parse(const std::string& filename);
};

#endif