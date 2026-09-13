#include "HttpUtils.hpp"
#include <string>

std::string	toLower(const std::string &key) {
	std::string	res = key;
	for (size_t i = 0; i < res.size(); i++)
		res[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(res[i])));
	return (res);
}

std::string	ft_int_to_string(int n) {
	std::string	res;
	bool		isNegative = false;

	if (n == 0)
		return "0";
	if (n < 0) {
		isNegative = true;
		n = -n;
	}
	while (n > 0) {
		res.insert(res.begin(), static_cast<char>('0' + n % 10));
		n /= 10;
	}
	if (isNegative)
		res.insert(res.begin(), '-');
	return (res);
}

std::string	createPrettyErrorPage(int code) {
	std::string	page;

	page += "<html><head><title>";
	page += ft_int_to_string(code);
	page += " ";
	page += "</title></head><body><h1>";
	switch (code) {
		case 400:
			page += "400 Bad Request";
			break;
		case 401:
			page += "401 Unauthorized";
			break;
		case 403:
			page += "403 Forbidden";
			break;
		case 404:
			page += "404 Not Found";
			break;
		case 405:
			page += "405 Method Not Allowed";
			break;
		case 413:
			page += "413 Payload Too Large";
			break;
		case 414:
			page += "414 URI Too Long";
			break;
		case 415:
			page += "415 Unsupported Media Type";
			break;
		case 500:
			page += "500 Internal Server Error";
			break;
		case 501:
			page += "501 Not Implemented";
			break;
		case 502:
			page += "502 Bad Gateway";
			break;
		case 503:
			page += "503 Service Unavailable";
			break;
		case 504:
			page += "504 Gateway Timeout";
			break;
		case 505:
			page += "505 HTTP Version Not Supported";
			break;
		case 508:
			page += "508 Loop Detected";
			break;
		default:
			page += ft_int_to_string(code);
			page += " Unknown Error";
	}
	page += "</h1></body></html>";
	return (page);
}
