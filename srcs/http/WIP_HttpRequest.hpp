#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

class	HttpRequest {
	private:
		std::string	method;
		std::string	path;
		std::string	query;
		std::string	version;
		std::string	body;
		std::map<std::string, std::string>	headers;
};

#endif
