#include "Connection.hpp"

Connection::Connection() :
				fd(-1),
				readBuffer(),
				writeBuffer(),
				httpVersion(),
				keepAlive(true),
				shouldClose(false),
				lastActivity(std::time(NULL))
				{}

Connection::Connection(int clientFd) : 
							fd(clientFd),
							readBuffer(),
							writeBuffer(),
							httpVersion(),
							keepAlive(true),
							shouldClose(false),
							lastActivity(std::time(NULL))
							{}