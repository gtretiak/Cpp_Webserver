#include "HttpResponse.hpp"
#include "StatusCodes.hpp"
#include <string>
#include <sstream>
#include <map>

HttpResponse::HttpResponse() : version_("HTTP/1.1"), statusCode_(200) {}

std::string	HttpResponse::toString() const {
	std::ostringstream	ss;
	ss << this->version_ << " " << this->statusCode_ << " "
	<< StatusCodes::getStatus(this->statusCode_) << "\r\n"
	ss = version + " " + ss + " ";
	ss += this->statusText_ + "\r\n";
       	for (std::map<std::string, std::string>::const_iterator i = this->headers_.begin(); i != this->headers_.end(); i++)
		res << i->first << " " << i->second << "\r\n";
	ss << "\r\n";
	ss << this->body_;
	return (ss.str());
}

void	setVersion(const std::string &v) {
	this->version_ = v;
}
void	setStatus(int code) {
	this->statusCode_ = code;
	this->statusText_ = StatusCodes::getStatus(code);
}
void	setBody(const std::string &b) {
	this->body_ = b;
}
void	setHeader(const std::string &k, const std::string &v) {
	this->headers_[k] = v;
}

HttpResponse::~HttpResponse() {}
