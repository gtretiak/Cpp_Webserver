#include "HttpResponse.hpp"
#include "StatusCodes.hpp"
#include <string>


HttpResponse::HttpResponse() {}

std::string	HttpResponse::toString() const {
	StatusCodes	sc;
	std::string	res;
	res = req->version + " " + std::itoa(this->statusCode) + " ";
	res += sc.getStatus(this->statusCode) + "\r\n";
       	for (every header inside headers)
		res += this->headers + "\r\n";
	res += "\r\n";
	res += this->body;
	return (res);
}
