#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <map>
#include <string>

class	HttpRequest {
	public:
		std::string	method;
		std::string	path;
		std::string	query;
		std::string	version;
		std::string	body;
		std::map<std::string, std::string>	headers;
};

#endif
