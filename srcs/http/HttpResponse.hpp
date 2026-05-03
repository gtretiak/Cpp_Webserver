#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <string>
#include <map>

class	HttpResponse {
	private:
		std::string	version_;
		int	statusCode_;
		std::string	statusText_;
		std::string	body_;
		std::map<std::string, std::string>	headers_;
	public:
		HttpResponse();
		std::string	toString() const;
		void	setVersion(const std::string &v);
		void	setStatus(int code);
		void	setBody(const std::string &b);
		void	setHeader(const std::string &k, const std::string &v);
		~HttpResponse();
};

#endif
