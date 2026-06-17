#include "HttpRequest.hpp"
#include "HttpException.hpp"

HttpRequest::HttpRequest() : redirectCount_(0) {}

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
static std::string	toLower(const std::string &key) {
	std::string     res = key;
	for (size_t i = 0; i < res.size(); i++)
		res[i] = std::tolower(res[i]);
	return (res);
}
void	HttpRequest::setHeader(const std::string &k, const std::string &v) {
	std::string	lowKey = toLower(k);
	if (lowKey == "host" || lowKey == "content-length" || lowKey == "content-type"
		|| lowKey == "date" || lowKey == "authorization" || lowKey == "server"
		|| lowKey == "location" || lowKey == "cache-control")
	{
		if (this->hasHeader(lowKey))
			throw HttpException(400, "Unique Header Duplicated: " + k);
	}
	this->headers_[lowKey] = v;
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
	std::string	lowKey = toLower(key);
	std::map<std::string, std::string>::const_iterator i = this->headers_.find(lowKey);
	if (i == this->headers_.end())
		return "";
		//throw HttpException(500, "Header Not Found");
	return (i->second);
}
const std::map<std::string, std::string>	&HttpRequest::getHeaders() const {
	return (this->headers_);
}
bool	HttpRequest::hasHeader(const std::string &key) const {
	std::string	lowKey = toLower(key);
	return (this->headers_.find(lowKey) != this->headers_.end());
}

void	HttpRequest::incrementRedirectCount( ) {
	redirectCount_++;
}

int	HttpRequest::getRedirectCount( ) const {
	return redirectCount_;
}

void	HttpRequest::setRedirectCount( int value ) {
	redirectCount_ = value;
}

HttpRequest::~HttpRequest() {}
