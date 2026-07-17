#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP

#include "../http/HttpResponse.hpp"
#include <string>

class	HttpException : public std::exception {
	private:
		int		code_;
		std::string	msg_;
	public:
		HttpException(int code, const std::string &msg);
		const char* what() const throw();
		int	code() const;
		virtual ~HttpException() throw() {};

		//void	errorCodeDispatcher(int code, HttpResponse& res);
};

#endif
