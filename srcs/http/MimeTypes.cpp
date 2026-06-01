#include "MimeTypes.hpp"
#include "HttpException.hpp"
#include <iostream>

MimeTypes::MimeTypes() {}
MimeTypes::~MimeTypes() {}

std::string	MimeTypes::getMimeType(const std::string &ext) {
	if (ext == ".html")
		return "text/html"; //: type (broad category) / subtype (specific format)
	else if (ext == ".css")
		return "text/css";
	else if (ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg"; 
	else if (ext == ".png")
		return "image/png";
	else if (ext == ".gif")
		return "image/gif";
	else if (ext == ".svg")
		return "image/svg+xml";
	else if (ext == ".ico")
		return "image/x-icon";
	else if (ext == ".txt")
		return "text/plain";
	else if (ext == ".json")
		return "application/json"; // application means 'some application-specific data'
	else if (ext == ".js")
		return "application/javascript";
	else if (ext == ".pdf")
		return "application/pdf";
	else // unknown format - raw bits - browser would offer to download it
		return "application/octet-stream";// raw data
}
