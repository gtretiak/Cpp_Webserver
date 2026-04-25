#include "HttpParser.hpp"

bool	HttpParser::isRequestComplete(const Connection &conn) const {
	std::string	&buf = conn.readBuffer;
	size_t	headerEnd = buf.find("\r\n\r\n"); // header completeness
	if (headerEnd == std::string::npos)
		return (false);
	std::string	header = buf.substr(0, headerEnd);

	if (GET or DELETE) - no body (request is complete)
	if (Content-Length)
		received body size should equal;
	else if (Transfer-Encoding: chunked)
		looking for '0\r\n\r\n';
	else
		?;
	return (true);
}
