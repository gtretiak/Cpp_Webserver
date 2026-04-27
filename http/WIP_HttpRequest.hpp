#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

class	HttpRequest {
	private:
		std::string	method;
		std::string	path;
		std::string	body;
		std::string	*headers;
};

#endif
