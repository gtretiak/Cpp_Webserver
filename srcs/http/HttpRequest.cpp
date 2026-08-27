#include "HttpRequest.hpp"
#include "HttpUtils.hpp"
#include "HttpException.hpp"

HttpRequest::HttpRequest() {
	method_ = "";
	url_ = "";
	path_ = "";
	query_ = "";
	version_ = "";
	body_ = "";
	headers_.clear();
	redirectCount_ = 0;
}

HttpRequest::HttpRequest( const HttpRequest& other ) : method_(other.method_),
	url_(other.url_), path_(other.path_), query_(other.query_), version_(other.version_),
	body_(other.body_), redirectCount_(other.redirectCount_), headers_(other.headers_) {
}

HttpRequest&	HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) {
		method_ = other.method_;
		url_ = other.url_;
		path_ = other.path_;
		query_ = other.query_;
		version_ = other.version_;
		body_ = other.body_;
		redirectCount_ = other.redirectCount_;
		headers_ = other.headers_;
	}
	return *this;
}

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
void	HttpRequest::setBody(std::string b) {
		body_.swap(b);
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
const std::string&	HttpRequest::getMethod() const {
	return (this->method_);
}
const std::string&	HttpRequest::getUrl() const {
	return (this->url_);
}
const std::string&	HttpRequest::getPath() const {
	return (this->path_);
}
const std::string&	HttpRequest::getQuery() const {
	return (this->query_);
}
const std::string&	HttpRequest::getVersion() const {
	return (this->version_);
}
const std::string&	HttpRequest::getBody() const {
	return (body_);
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
