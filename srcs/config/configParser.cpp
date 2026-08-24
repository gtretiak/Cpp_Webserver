/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 09:18:39 by dopereir          #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2026/08/24 10:46:10 by dopereir         ###   ########.fr       */
=======
/*   Updated: 2026/08/22 17:39:01 by dopereir         ###   ########.fr       */
>>>>>>> 2ecfc257a9be727c22676c29f89b24b0c9f72900
/*                                                                            */
/* ************************************************************************** */

#include "configParser.hpp"

# include <cctype>
# include <cstdlib>
# include <fstream>
# include <sstream>
# include <stdexcept>

configParser::configParser() : _tokens(), _pos(0) {}

configParser::~configParser() {
}

configParser::Token configParser::peek() const {
	if (_pos >= _tokens.size()) {
		Token token;
		token.value = "";
		token.line = _tokens.empty() ? 1 : _tokens.back().line;
		return token;
	}
	return _tokens[_pos];
}

const configParser::Token& configParser::current() const {
	if (_pos >= _tokens.size()) {
		throw parseError("Unexpected end of configuration");
	}
	return _tokens[_pos];
}

bool configParser::eof() const {
	return (_pos >= _tokens.size());
}

bool configParser::isSymbol(const std::string& value) const {
	return (!eof() && _tokens[_pos].value == value);
}

configParser::Token configParser::consume() {
	if (eof()) {
		throw parseError("Unexpected end of configuration");
	}
	return _tokens[_pos++];
}

void configParser::expect(const std::string& value) {
	if (eof() || _tokens[_pos].value != value) {
		throw parseError(formatError(eof() ? (_tokens.empty() ? 1 : _tokens.back().line) : _tokens[_pos].line,
								"Expected '" + value + "'"));
	}
	++_pos;
}

std::string configParser::formatError(size_t line, const std::string& message) const {
	std::ostringstream out;
	out << "Config parse error at line " << line << ": " << message;
	return out.str();
}

bool	configParser::isNumber(const std::string& value) const {
	if (value.empty()) {
		return false;
	}
	for (size_t i = 0; i < value.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
			return false;
		}
	}
	return true;
}

bool	configParser::isIPv4(const std::string& value) const {
	std::string::size_type	start = 0;
	int						dots = 0;
	int						octet;

	while (start <= value.size()) {
		std::string::size_type	end = value.find('.', start);
		std::string 			part = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
		if (part.empty() || !isNumber(part)) {
			return false;
		}
		octet = std::atoi(part.c_str());
		if (octet < 0 || octet > 255)
			return false;
		++dots;
		if (end == std::string::npos) {
			break;
		}
		start = end + 1;
	}
	return (dots == 4);
}

bool	configParser::isURL(const std::string& value) const {
	const std::string	http = "http://";
	const std::string	https = "https://";

	if (value.compare(0, http.size(), http) != 0 && value.compare(0, https.size(), https) != 0) {
		return false;
	}
	if ((value.compare(0, http.size(), http) == 0 && value.size() == http.size())
		|| (value.compare(0, https.size(), https) == 0 && value.size() == https.size())) {
		return false;
	}
	return true;
}

std::string	configParser::join(const std::vector<std::string>& values) const {
	std::ostringstream out;
	for (size_t i = 0; i < values.size(); ++i) {
		if (i != 0) {
			out << ' ';
		}
		out << values[i];
	}
	return out.str();
}

std::vector<configParser::Token>	configParser::tokenize(const std::string& content) const {
	std::vector<Token> tokens;
	std::string current;
	size_t line = 1;

	for (size_t i = 0; i < content.size(); ++i) {
		char c = content[i];
		if (c == '#') {
			if (!current.empty()) {
				Token token;
				token.value = current;
				token.line = line;
				tokens.push_back(token);
				current.clear();
			}
			while (i < content.size() && content[i] != '\n') {
				++i;
			}
			if (i < content.size() && content[i] == '\n') {
				++line;
			}
			continue;
		}
		if (c == '\n') {
			if (!current.empty()) {
				Token token;
				token.value = current;
				token.line = line;
				tokens.push_back(token);
				current.clear();
			}
			++line;
			continue;
		}
		if (c == '{' || c == '}' || c == ';') {
			if (!current.empty()) {
				Token token;
				token.value = current;
				token.line = line;
				tokens.push_back(token);
				current.clear();
			}
			Token token;
			token.value = std::string(1, c);
			token.line = line;
			tokens.push_back(token);
			continue;
		}
		if (std::isspace(static_cast<unsigned char>(c))) {
			if (!current.empty()) {
				Token token;
				token.value = current;
				token.line = line;
				tokens.push_back(token);
				current.clear();
			}
			continue;
		}
		current += c;
	}
	if (!current.empty()) {
		Token token;
		token.value = current;
		token.line = line;
		tokens.push_back(token);
	}
	return tokens;
}

std::vector<std::string>	configParser::collectArguments() {
	std::vector<std::string> args;
	while (!eof() && !isSymbol(";") && !isSymbol("{") && !isSymbol("}")) {
		args.push_back(consume().value);
	}
	return args;
}

Listen configParser::parseListen(const std::string& value, size_t line) const {
	Listen	listen;

	if (value.size() >= 5 && value.substr(0, 5) == "unix:") {
		listen.type = Listen::UNIX_SOCK;
		listen.addr = value.substr(5);
		listen.port = -1;
		if (listen.addr.empty()) {
			throw parseError(formatError(line, "Invalid unix socket path"));
		}
		return listen;
	}
	std::string::size_type	colon = value.rfind(':');
	if (colon != std::string::npos) {
		std::string	host = value.substr(0, colon);
		std::string	port = value.substr(colon + 1);
		if (!isNumber(port) || std::atoi(port.c_str()) < 1
				|| std::atoi(port.c_str()) > MAX_PORT_VAL) {
			throw parseError(formatError(line, "Invalid listen port"));
		}
		listen.port = std::atoi(port.c_str());
		listen.addr = host;
		if (host == "*")
			listen.type = Listen::IP_PORT;
		else if (isIPv4(host))
			listen.type = Listen::IP_PORT;
		else
			listen.type = Listen::HOST_PORT;
		return listen;
	}
	if (isIPv4(value)) {
		listen.port = 80;
		listen.addr = value;
		listen.type = Listen::IP_PORT;
		return listen;
	}
	if (!isNumber(value)) {
		throw parseError(formatError(line, "Invalid listen directive"));
	}
	listen.type = Listen::PORT;
	listen.port = std::atoi(value.c_str());
	if (listen.port < 0 || listen.port > MAX_PORT_VAL)
		throw parseError(formatError(line, "Invalid port number"));
	return listen;
}

size_t configParser::parseSize(const std::string& value, size_t line) const {
	if (value.empty()) {
		throw parseError(formatError(line, "Invalid size value"));
	}
	size_t i = 0;
	while (i < value.size() && std::isdigit(static_cast<unsigned char>(value[i]))) {
		++i;
	}
	if (i == 0) {
		throw parseError(formatError(line, "Invalid size value"));
	}
	size_t number = static_cast<size_t>(std::strtoul(value.substr(0, i).c_str(), 0, 10));
	std::string suffix = value.substr(i);
	if (suffix.empty() || suffix == "b" || suffix == "B") {
		return number;
	}
	if (suffix == "k" || suffix == "K" || suffix == "kb" || suffix == "Kb" || suffix == "kB" || suffix == "KB") {
		return number * 1024;
	}
	if (suffix == "m" || suffix == "M" || suffix == "mb" || suffix == "Mb" || suffix == "mB" || suffix == "MB") {
		return number * 1024 * 1024;
	}
	if (suffix == "g" || suffix == "G" || suffix == "gb" || suffix == "Gb" || suffix == "gB" || suffix == "GB") {
		return number * 1024 * 1024 * 1024;
	}
	throw parseError(formatError(line, "Unknown size suffix: " + suffix));
}

std::map<int, std::string>	configParser::parseReturn(const std::vector<std::string> &value, size_t line) const {
	std::map<int, std::string>	_return_redirect;
	int							status_code = 302;

	if (value.empty() || value.size() > 2) {
		throw	parseError(formatError(line, "return directive has too many arguments"));
	}
	if (value.size() == 1) {
		if (isNumber(value[0])) {
			status_code = std::atoi(value[0].c_str());
			_return_redirect[status_code] = "";
			return _return_redirect;
		}
		if (isURL(value[0])) {
			_return_redirect[status_code] = value[0];
			return _return_redirect;
		}
		throw parseError(formatError(line, "invalid return directive"));
	}
	if (!isNumber(value[0])) {
		throw parseError(formatError(line, "invalid return directive"));
	}
	status_code = std::atoi(value[0].c_str());
	if (status_code >= 300 && status_code <= 308) {
		if (isURL(value[1])) {
			_return_redirect[status_code] = value[1];
		}
		else {
			_return_redirect[status_code] = value[1];
		} 
	}
	else {
		_return_redirect[status_code] = value[1];
	}
	return _return_redirect;
}

void configParser::appendAllowedMethod(limitExcept& methods, const std::string& value) {
	if (value == "GET") {
		methods.GET = true;
	}
	else if (value == "POST") {
		methods.POST = true;
	}
	else if (value == "DELETE") {
		methods.DELETE = true;
	}
}

void configParser::applyServerDirective(serverConfig& server,
								const std::string& name,
								const std::vector<std::string>& args,
								size_t line) {
	directiveValue	raw;
	raw.name = name;
	raw.args = args;
	server._directives.push_back(raw);

	if (name == "listen") {
		if (args.empty()) {
			throw parseError(formatError(line, "listen directive requires at least one argument"));
		}
		server._listens.push_back(parseListen(args[0], line));
	}
	else if (name == "server_name") {
		server._server_names.insert(server._server_names.end(), args.begin(), args.end());
	}
	else if (name == "root") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "root directive expects one argument"));
		}
		server._root = args[0];
	}
	else if (name == "alias") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "alias directive expects one argument"));
		}
		server._alias = args[0];
	}
	else if (name == "index") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "index directive expects only one argument"));
		}
		server._index.insert(server._index.end(), args[0]);
	}
	else if (name == "autoindex") {
		if (args.size() != 1 || (args[0] != "on" && args[0] != "off")) {
			throw parseError(formatError(line, "autoindex expects 'on' or 'off'"));
		}
		server._autoindex = (args[0] == "on");
	}
	else if (name == "client_max_body_size") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "client_max_body_size expects one argument"));
		}
		server._client_max_body_size = parseSize(args[0], line);
		if (server._client_max_body_size == 0) {
			server._has_client_max_body_size = false;
		}
		else
			server._has_client_max_body_size = true;
	}
	else if (name == "error_page") {
		if (args.size() < 2) {
			throw parseError(formatError(line, "error_page expects at least one code and one path"));
		}
		std::string path = args.back();
		for (size_t i = 0; i + 1 < args.size(); ++i) {
			if (!isNumber(args[i])) {
				throw parseError(formatError(line, "error_page codes must be numeric"));
			}
			server._error_pages[std::atoi(args[i].c_str())] = path;
		}
	}
	else if (name == "allowed_methods") {
		if (args.empty()) {
			throw parseError(formatError(line, "allowed_methods expects at least one argument"));
		}
		server._allowed_methods.GET = false;
		server._allowed_methods.POST = false;
		server._allowed_methods.DELETE = false;
		server._has_limit_except = true;
		for (size_t i = 0; i < args.size(); ++i) {
			appendAllowedMethod(server._allowed_methods, args[i]);
		}
	}
	else if (name == "cgi_extension") {
		if (args.size() < 2) {
			throw parseError(formatError(line, "cgi_extension expects extension and executable path"));
		}
		if (!isValidCgiExtention(args[0])) {
			throw parseError(formatError(line, "Invalid CGI extension"));
		}
		cgiExecutableConf	cgiExe;
		cgiExe.path = args[1];

		if (args.size() > 2) {//has method restriction
			parseCgiExtension(args, cgiExe.allowedMethodsCGI);
		}
		server._cgi.cgi_extension[args[0]] = cgiExe;
		server._has_cgi = true;
	}
	else if (name == "return") {
		if (args.empty()) {
			throw parseError(formatError(line, "return directive expects at least one argument"));
		}
		if (!server._has_return) {
			server._return = parseReturn(args, line);
			server._has_return = true;
		}
	}
	else if (name == "upload_store") {
		if (args.size() != 1)
			throw parseError(formatError(line, "upload_store expects a path, the field cannot be empty"));
		server._upload_store = args[0];
	}
}

void configParser::applyLocationDirective(locationConfig& location,
								const std::string& name,
								const std::vector<std::string>& args,
								size_t line) {
	directiveValue raw;
	raw.name = name;
	raw.args = args;
	location._directives.push_back(raw);

	if (name == "root") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "root directive expects one argument"));
		}
		location._root = args[0];
	}
	else if (name == "alias") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "alias directive expects one argument"));
		}
		location._alias = args[0];
	}
	else if (name == "index") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "index directive expects one argument"));
		}
		location._index.insert(location._index.end(), args[0]);
	}
	else if (name == "autoindex") {
		if (args.size() != 1 || (args[0] != "on" && args[0] != "off")) {
			throw parseError(formatError(line, "autoindex expects 'on' or 'off'"));
		}
		location._autoindex = (args[0] == "on");
	}
	else if (name == "client_max_body_size") {
		if (args.size() != 1) {
			throw parseError(formatError(line, "client_max_body_size expects one argument"));
		}
		location._client_max_body_size = parseSize(args[0], line);
		if (location._client_max_body_size == 0) {
			location._has_client_max_body_size = false;
		}
		else
			location._has_client_max_body_size = true;
	}
	else if (name == "error_page") {
		if (args.size() < 2) {
			throw parseError(formatError(line, "error_page expects at least one code and one path"));
		}
		std::string path = args.back();
		for (size_t i = 0; i + 1 < args.size(); ++i) {
			if (!isNumber(args[i])) {
				throw parseError(formatError(line, "error_page codes must be numeric"));
			}
			location._error_pages[std::atoi(args[i].c_str())] = path;
		}
	}
	else if (name == "try_files") {
		location._try_files.insert(location._try_files.end(), args.begin(), args.end());
	}
	else if (name == "allowed_methods") {
		if (args.empty()) {
			throw parseError(formatError(line, "allowed_methods expects at least one argument"));
		}
		location._allowed_methods.GET = false;
		location._allowed_methods.POST = false;
		location._allowed_methods.DELETE = false;
		location._has_limit_except = true;
		for (size_t i = 0; i < args.size(); ++i) {
			appendAllowedMethod(location._allowed_methods, args[i]);
		}
	}
	else if (name == "cgi_extension") {
		if (args.size() < 2) {
			throw parseError(formatError(line, "cgi_extension expects extension and executable path"));
		}
		if (!isValidCgiExtention(args[0])) {
			throw parseError(formatError(line, "Invalid CGI extension"));
		}
		cgiExecutableConf	cgiExe;
		cgiExe.path = args[1];

		if (args.size() > 2) {//has method restriction
			parseCgiExtension(args, cgiExe.allowedMethodsCGI);
		}
		location._cgi.cgi_extension[args[0]] = cgiExe;
		location._has_cgi = true;
	}
	else if (name == "upload_store") {
		if (args.size() != 1)
			throw parseError(formatError(line, "upload_store expects a path, the field cannot be empty"));
		location.upload_store = args[0];
	}
	else if (name == "return") {
		if (args.empty()) {
			throw parseError(formatError(line, "return directive expects at least one argument"));
		}
		if (!location._has_return) {
			location._return = parseReturn(args, line);
			location._has_return = true;
		}
	}
}

serverConfig configParser::parseServerBlock() {
	expect("server");
	expect("{");

	serverConfig server;
	while (!eof() && !isSymbol("}")) {
		Token token = peek();
		if (token.value == "location") {
			server._locations.push_back(parseLocationBlock());
		}
		else {
			std::string name = consume().value;
			std::vector<std::string> args = collectArguments();
			expect(";");
			applyServerDirective(server, name, args, token.line);
		}
	}
	if (eof()) {
		throw parseError("Unexpected end of configuration while parsing server block");
	}
	expect("}");
	return server;
}

locationConfig configParser::parseLocationBlock() {
	expect("location");
	if (eof()) {
		throw parseError("Unexpected end of configuration after location");
	}
	Token pathToken = consume();
	if (pathToken.value == "{" || pathToken.value == "}" || pathToken.value == ";") {
		throw parseError(formatError(pathToken.line, "location requires a path argument"));
	}
	expect("{");

	locationConfig location;
	location._path = pathToken.value;
	while (!eof() && !isSymbol("}")) {
		Token token = peek();
		std::string name = consume().value;
		std::vector<std::string> args = collectArguments();
		expect(";");
		applyLocationDirective(location, name, args, token.line);
	}
	if (eof()) {
		throw parseError("Unexpected end of configuration while parsing location block");
	}
	expect("}");
	return location;
}

globalConfig configParser::parseConfig() {
	globalConfig	config;

	while (!eof()) {
		Token token = peek();
		if (token.value == "server") {
			config.servers.push_back(parseServerBlock());
		}
		else {
			throw parseError(formatError(token.line, "Expected 'server' block"));
		}
	}
	return config;
}

void	configParser::parseCgiExtension( const std::vector<std::string>& args, limitExcept& methods) {
	methods.GET = false;
	methods.POST = false;
	methods.DELETE = false;

	// Iterate through all arguments after the executable path (index 2 onwards)
	for (size_t i = 2; i < args.size(); ++i) {
		std::string token = args[i];

		for (size_t j = 0; j < token.length(); ++j) {
			token[j] = std::toupper(token[j]);
		}

		if (token == "|") continue;

		std::istringstream iss(token);
		std::string method;
		while (std::getline(iss, method, '|')) {
			if (method.empty()) continue;
			
			if (method == "GET") methods.GET = true;
			else if (method == "POST") methods.POST = true;
			else if (method == "DELETE") methods.DELETE = true;
			else {
				throw std::runtime_error("Invalid method in cgi_extension: " + method);
			}
		}
	}
}

globalConfig configParser::parse(const std::string& filename) {
	std::ifstream	file(filename.c_str(), std::ios::in | std::ios::binary);
	if (!file) {
		throw parseError("Unable to open configuration file: " + filename);
	}
	std::ostringstream	buffer;

	buffer << file.rdbuf();
	_tokens = tokenize(buffer.str());
	_pos = 0;
	return parseConfig();
}
