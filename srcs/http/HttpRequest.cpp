#include "HttpRequest.hpp"
#include "HttpException.hpp"

HttpRequest::HttpRequest() {}
void	HttpRequest::setMethod(const std::string &m) {
	this->method_ = m;
}
void	HttpRequest::setUrl(const std::string &u) {
	this->url_ = u;
}
void	HttpRequest::setPath(const std::string &p) {
	this->path_ = p;
}
void	HttpRequest::setQuery(const std::string &q) {
	this->query_ = q;
}
void	HttpRequest::setVersion(const std::string &v) {
	this->version_ = v;
}
void	HttpRequest::setBody(const std::string &b) {
	this->body_ = b;
}
void	HttpRequest::setHeader(const std::string &k, const std::string &v) {
	this->headers_[k] = v;
}
std::string	HttpRequest::getMethod() const {
	return (this->method_);
}
std::string	HttpRequest::getUrl() const {
	return (this->url_);
}
std::string	HttpRequest::getPath() const {
	return (this->path_);
}
std::string	HttpRequest::getQuery() const {
	return (this->query_);
}
std::string	HttpRequest::getVersion() const {
	return (this->version_);
}
std::string	HttpRequest::getBody() const {
	return (this->body_);
}
std::string	HttpRequest::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator i = this->headers_.find(key);
	if (i == this->headers_.end())
		throw HttpException(500, "Header Not Found");
	return (i->second);
}
const std::map<std::string, std::string>	&HttpRequest::getHeaders() const {
	return (this->headers_);
}
bool	HttpRequest::hasHeader(const std::string &key) const {
	return (this->headers_.find(key) != this->headers_.end());
}

HttpRequest::~HttpRequest() {}
