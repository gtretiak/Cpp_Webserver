#include "HttpResponse.hpp"
#include "HttpException.hpp"
#include "StatusCodes.hpp"
#include <string>
#include <sstream>
#include <cstring>//char string functions - strftime()
#include <iomanip>
#include <map>
//#include <ctime>//time_t, time(), gmtime()

static std::string	getCurrentDate() {
/*	time_t	now = time(NULL);
	struct tm	*timeinfo = gmtime(&now);
	char	buf[100];
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y, %H:%M:%S GMT", timeinfo);
	return (std::string(buf));*/
	return ("Wed 21 Oct 2067 14:44:44 GMT");
}

HttpResponse::HttpResponse() : version_("HTTP/1.1"), statusCode_(200), statusText_(StatusCodes::getStatus(200)) {
	this->setHeader("Date", getCurrentDate());
	this->setHeader("Server", "Our Server");
}

HttpResponse::HttpResponse(const std::string &serverName) : version_("HTTP/1.1"), serverName_(serverName), statusCode_(200), statusText_(StatusCodes::getStatus(200)) {
	this->setHeader("Date", getCurrentDate());
	this->setHeader("Server", serverName_);
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

int	HttpResponse::getStatusCode() {
	return (this->statusCode_);
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
std::string	HttpResponse::getHeader(const std::string &k) const {
	std::map<std::string, std::string>::const_iterator i = this->headers_.find(k);
	if (i == this->headers_.end())
		throw HttpException(400, "Header Not Found");
	return (i->second);
}
bool	HttpResponse::hasHeader(const std::string &k) const {
	return (this->headers_.find(k) != this->headers_.end());
}

HttpResponse::~HttpResponse() {}
