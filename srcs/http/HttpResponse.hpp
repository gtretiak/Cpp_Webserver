#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

class	HttpResponse {
	public:
		HttpResponse();
		HttpResponse(int statusCode, const std::string &data);
		int					statusCode;
		std::string				statusText;
		std::string				body;
		std::map<std::string, std::string>	headers;
		std::string	toString() const;
		void	setHeader(const std::string &k, const std::string &v);
};

#endif
