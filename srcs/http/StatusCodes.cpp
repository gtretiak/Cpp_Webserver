#include "StatusCodes.hpp"

static const std::string	&StatusCodes::getStatus(int code) {
	switch (code)
	{
		//1xx informational
		case 100: return "Continue";//headers received, body expected
		//2xx success
		case 200: return "OK";
		case 201: return "Created"; //upload/POST success
		case 204: return "No Content";//success, but no body
		//3xx redirection
		case 301: return "Moved Permanently";//redirect (permanent)
		case 302: return "Found";//redirect (temporary)
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		//4xx client error
		case 400: return "Bad Request";//malformed request
		case 403: return "Forbidden";//no permission
		case 404: return "Not Found";//resource doesn't exist
		case 405: return "Method Not Allowed";
		case 408: return "Request Timeout";//client too slow
		case 413: return "Payload Too Large";//body exceeds maxSize
		//5xx server error
		case 500: return "Internal Server Error";//unexpected logic err
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";//CGI returned invalid response
		case 504: return "Gateway Timeout";//CGI didn't respond in time
		default: return "Unknown";
	}
}
