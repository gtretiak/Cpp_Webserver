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
/*#include <ctime>//time_t, time(), gmtime()
static std::string	getCurrentDate() {
	std::time_t	now = std::time(NULL);
	std::tm		*timeinfo = std::gmtime(&now);
	char		buf[100];

	if (!timeinfo)
		return ("Thu, 01 Jan 1970 00:00:00 GMT");
	if (std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", timeinfo) == 0)
		return ("Thu, 01 Jan 1970 00:00:00 GMT");
	return (std::string(buf));
}*/

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
	return this->version_;
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

void	HttpResponse::generateErrorPageResponse(const char *filepath) {
	char	*buffer = new char[BUFFER_SIZE];
	int	fd = open(filepath, O_RDONLY);
	if (fd == -1)
	{
		delete[] buffer;
		this->body_ = "<html><body><h1>"
			"404 Not Found"
			"</h1></body></html>";
		this->setBody(this->body_);	
		return ;
	}
	ssize_t	bytes = 1;

	//std::cout << "****** debug: generateErrorPageResponse: filepath = " << filepath << std::endl;TODO - remove before submitting
	while (bytes > 0)
	{
		bytes = read(fd, buffer, sizeof(buffer));
		if (bytes <= 0)
			break ;
		this->body_.append(buffer, n);
	}
//	std::cout << "****** debug: generateErrorPageResponse: body = " << this->body_ << std::endl;TODO - remove before submitting
	std::ostringstream	ss;
	ss << this->body_.length();
	delete[] buffer;
	this->setVersion("HTTP/1.1");
	this->setHeader("Content-Length", ss.str());
	this->setHeader("Content-Type", "text/html");
	close(fd);
}
