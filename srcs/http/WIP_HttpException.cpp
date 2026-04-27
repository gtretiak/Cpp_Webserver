#include "HttpException.hpp"

class	HttpException : public std::exception {
	private:
		int		code_;
		std::string	msg_;
HttpException::HttpException(int code, const std::string &msg) : code_(code), msg_(msg) {}
const char	*HttpException::what() const throw() {
}
int	HttpException::code() const {
}
