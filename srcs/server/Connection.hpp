#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include <string>
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../http/HttpException.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include <poll.h>
#include <cstdlib>
#include <unistd.h>
#include "../config/serverConfig.hpp"
#include "../config/locationConfig.hpp"

enum ConnectionState {
	READING,//readClient, accumulate bytes from client socket
	RUNNING,//child executes, accumlate bytes from cgi script stdout
	WRITING,//child finished, build response, convert to string and send to client socket
	CLOSING //close client socket, remove from pollfd vector, remove from connections map
};

typedef struct CgiContext {
	int			inFd;		// fd to read from the CGI script
	int			outFd;		// fd to write to the CGI script
	pid_t		pid;		// pid of the CGI process
	std::string	outputBuffer;	// buffer to store the CGI output
	time_t		cgiLastActivity;	// timestamp of the last activity for timeout handling
	struct pollfd	pollFd;		// pollfd structure for monitoring the CGI script's output

	CgiContext() : inFd(-1), outFd(-1), pid(-1), outputBuffer(""), cgiLastActivity(0) {
		pollFd.fd = -1;
		pollFd.events = POLLIN;
		pollFd.revents = 0;
	}
	CgiContext(const CgiContext& other) : inFd(other.inFd),
		outFd(other.outFd),
		pid(other.pid),
		outputBuffer(other.outputBuffer),
		cgiLastActivity(other.cgiLastActivity),
		pollFd(other.pollFd) {}
	CgiContext& operator=(const CgiContext& other) {
		if (this != &other) {
			inFd = other.inFd;
			outFd = other.outFd;
			pid = other.pid;
			outputBuffer = other.outputBuffer;
			cgiLastActivity = other.cgiLastActivity;
			pollFd = other.pollFd;
		}
		return *this;
	}
}	CgiContext;


class	Connection : public CgiRequestHandler {
	int	fd;						// client fd
	int	serverIndex;
	serverConfig	*matchedServer;
	locationConfig	*matchedLocation;
	std::string	readBuffer;		// bytes received bu recv()
	std::string	writeBuffer;	// bytes pending to send()
	std::string	httpVersion;	// filled after by parser
	bool		keepAlive;		// indicates if the connection can continue open
	bool		shouldClose;	// marks connection to close after writing
	time_t		lastActivity;	// used later to timeout

	HttpRequest	req;
	HttpResponse	res;

	ConnectionState	state;		// current state of the connection
	CgiContext		cgiData;	// context for CGI handling

	Connection();
	Connection(int clientFd);
	Connection(const Connection& other);
	Connection& operator=(const Connection& other);

	void	clear();
	void	resetConnection();
};

void	writeRequestBodyToCgi( HttpRequest &req, int stdin_fd );


#endif
