#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include <string>

struct Connection
{
	int fd;					 // client fd
	std::string readBuffer;	 // bytes received bu recv()
	std::string writeBuffer; // bytes pending to send()
	std::string httpVersion; // filled after by parser
	bool keepAlive;			 // indicates if the connection can continue open
	bool shouldClose;		 // marks connection to close after writing
	time_t lastActivity;	 // used later to timeout

	Connection();
	Connection(int clientFd);
};

#endif