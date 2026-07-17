#include "HttpException.hpp"

HttpException::HttpException(int code, const std::string &msg) : code_(code), msg_(msg) {}
const char	*HttpException::what() const throw() {
	return (this->msg_.c_str());
}
int	HttpException::code() const {
	return (this->code_);
}
/*
/// @brief Build response based on code
/// @param code 
/// @param res 
void	HttpException::errorCodeDispatcher(int code, HttpResponse& res) {

}
*/