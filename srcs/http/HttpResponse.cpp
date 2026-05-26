#include "HttpResponse.hpp"
#include "StatusCodes.hpp"
#include <string>
#include <sstream>
#include <map>

HttpResponse::HttpResponse() : version_("HTTP/1.1"), statusCode_(200), statusText_(StatusCodes::getStatus(200)) {
}

std::string	HttpResponse::toString() const {
	std::ostringstream	ss;
	ss << this->version_ << " " << this->statusCode_ << " ";
	ss << this->statusText_ << "\r\n";
       	if (!this->headers_.empty())
	{
		for (std::map<std::string, std::string>::const_iterator i = this->headers_.begin(); i != this->headers_.end(); i++)
		ss << i->first << ": " << i->second << "\r\n";
	}
	if (!this->body_.empty())
	{
		ss << "\r\n";
		ss << this->body_;
	}
	return (ss.str());
}
void	HttpResponse::setVersion(const std::string &v) {
	this->version_ = v;
}
void	HttpResponse::setStatus(int code) {
// 204, 304?
	this->statusCode_ = code;
	this->statusText_ = StatusCodes::getStatus(code);
}
void	HttpResponse::setBody(const std::string &b) {
	std::ostringstream	ss;
	ss << b.length();
	this->setHeader("Content-Length", ss.str());
	this->body_ = b;
}
void	HttpResponse::setHeader(const std::string &k, const std::string &v) {
	this->headers_[k] = v;
}
//keep alive logic TODO

HttpResponse::~HttpResponse() {}
