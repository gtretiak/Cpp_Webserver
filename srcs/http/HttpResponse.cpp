#include "HttpResponse.hpp"
#include "HttpException.hpp"
#include "HttpUtils.hpp"
#include "StatusCodes.hpp"
#include <string>
#include <sstream>
#include <ctime>
#include <cctype>
#include <iomanip>
#include <map>

HttpResponse::HttpResponse() : version_("HTTP/1.1"), statusCode_(200),	statusText_(StatusCodes::getStatus(200)), has_status_(false) {
//	date_ = getCurrentDate();TODO date remove or leave commented - forbidden
	//this->setHeader("server", "Our Server");
}

HttpResponse::HttpResponse(const std::string &serverName) : version_("HTTP/1.1"), serverName_(serverName), statusCode_(200), statusText_(StatusCodes::getStatus(200)), has_status_(false) {
//	date_ = getCurrentDate();TODO date remove or leave commented - forbidden
	//this->setHeader("server", serverName_);
}

HttpResponse&	HttpResponse::operator=(const HttpResponse &other) {
	if (this != &other) {
		version_ = other.version_;
		serverName_ = other.serverName_;
		statusCode_ = other.statusCode_;
		statusText_ = other.statusText_;
		body_ = other.body_;
//		date_ = other.date_;date remove or keep commented TODO
		headers_ = other.headers_;
		has_status_ = other.has_status_;
	}
	return *this;
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
	ss << "\r\n";
	if (!this->body_.empty())
		ss << this->body_;
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

void		HttpResponse::setStatusText( ) {
	this->statusText_ = StatusCodes::getStatus(statusCode_);
}

void	HttpResponse::setStatusBool( bool setting ) {
	has_status_ = setting;
}

bool	HttpResponse::hasStatusCode( ) {
	return has_status_;
}

int	HttpResponse::getStatusCode() {
	return (this->statusCode_);
}

void	HttpResponse::setBody(const std::string &b) {//MAYBE NOT SET CONTENT HERE
	std::ostringstream	ss;
	if (!b.empty())
	{
		this->body_ = b;
		ss << b.length();
	}
	else
		ss << 0;
	this->setHeader("content-length", ss.str());
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

const std::map<std::string, std::string>&	HttpResponse::getHeaders( ) const {
	return this->headers_;
}

const std::string	HttpResponse::getVersion( ) const {
	/*if (!version_.empty())
		return "";*/
	return version_;
}

const std::string	HttpResponse::getBody() const {
	if (body_.empty())
		return "";
	return body_;
}

bool	HttpResponse::hasHeader(const std::string &k) const {
	std::string	lowKey = toLower(k);
	return (this->headers_.find(lowKey) != this->headers_.end());
}

HttpResponse::~HttpResponse() {}

void	HttpResponse::generateErrorPageResponse( const char *filepath, int errorCode ) {
	std::ostringstream	ss;
	std::string			errorPage;
	int					fd;

	//check if the file exists and is readable
	if (filepath == NULL || access(filepath, F_OK | R_OK) == -1)
	{
		errorPage = createPrettyErrorPage(errorCode);
		this->setBody(errorPage);
		this->setVersion("HTTP/1.1");
		if (!this->hasHeader("content-type"))
			this->setHeader("Content-Type", "text/html");
		return ;
	}
	fd = open(filepath, O_RDONLY);
	//in case open fails, generate a default error page
	if (fd == -1)
	{
		errorPage = createPrettyErrorPage(errorCode);
		this->setBody(errorPage);
		this->setVersion("HTTP/1.1");
		if (!this->hasHeader("content-type"))
			this->setHeader("Content-Type", "text/html");
		return ;
	}

	char	buffer[4096];
	ssize_t	bytesRead;
	std::cout << "****** REMOVE ME LATER debug: generateErrorPageResponse: filepath = " << filepath << std::endl;
	//read file into buffer and append to body_
	while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[bytesRead] = '\0';
		this->body_.append(buffer, bytesRead);
	}
	std::cout << "****** REMOVE ME LATER debug: generateErrorPageResponse: body = " << this->body_ << std::endl;
	ss << this->body_.length();
	this->setVersion("HTTP/1.1");
	if (!this->hasHeader("content-length"))
		this->setHeader("Content-Length", ss.str());
	if (!this->hasHeader("content-type"))
		this->setHeader("Content-Type", "text/html");
	close(fd);
}
