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
	this->setHeader("date", getCurrentDate());
	this->setHeader("server", "Our Server");
}

HttpResponse::HttpResponse(const std::string &serverName) : version_("HTTP/1.1"), serverName_(serverName), statusCode_(200), statusText_(StatusCodes::getStatus(200)) {
	this->setHeader("date", getCurrentDate());
	this->setHeader("server", serverName_);
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
	this->setHeader("content-length", ss.str());
	this->body_ = b;
}
static std::string	toLower(const std::string &key) {
	std::string     res = key;
	for (size_t i = 0; i < res.size(); i++)
		res[i] = std::tolower(res[i]);
	return (res);
}

void	HttpResponse::setHeader(const std::string &k, const std::string &v) {
	std::string	lowKey = toLower(k);
	if (lowKey == "content-length" || lowKey == "content-type" || lowKey == "date" || lowKey == "server" || lowKey == "location")
	{
	       if (this->hasHeader(lowKey))
		       throw HttpException(400, "Unique Header Duplicated: " + k);
	}
	this->headers_[lowKey] = v;
}
std::string	HttpResponse::getHeader(const std::string &k) const {
	std::string	lowKey = toLower(k);
	std::map<std::string, std::string>::const_iterator i = this->headers_.find(lowKey);
	if (i == this->headers_.end())
		return "";
		//throw HttpException(400, "Header Not Found");
	return (i->second);
}
bool	HttpResponse::hasHeader(const std::string &k) const {
	std::string	lowKey = toLower(k);
	return (this->headers_.find(lowKey) != this->headers_.end());
}

HttpResponse::~HttpResponse() {}
