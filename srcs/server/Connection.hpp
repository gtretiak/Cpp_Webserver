#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>

struct Connection {
	std::string	readBuffer;
	std::string	writeBuffer;
	std::string	httpVersion;
	bool		keepAlive;
};

#endif
