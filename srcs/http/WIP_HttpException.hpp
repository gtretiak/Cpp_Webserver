#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP

class	HttpException : public std::exception {
	private:
		int		code_;
		std::string	msg_;
	public:
		HttpException(int code, const std::string &msg);
		const char* what() const throw();
		int code() const;
};

#endif
