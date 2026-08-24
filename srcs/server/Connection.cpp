#include "Connection.hpp"

Connection::Connection() : CgiRequestHandler(), matchedServer(NULL), matchedLocation(NULL)
{
	fd = -1;
	serverIndex = -1;
	readBuffer = "";
	writeBuffer = "";
	httpVersion = "";
	keepAlive = true;
	shouldClose = false;
	lastActivity = std::time(NULL);
	req = HttpRequest();
	res = HttpResponse();
	cgiExecutable = "";
	cgiData = CgiContext();
}

Connection::Connection(int clientFd) : CgiRequestHandler(), fd(clientFd), matchedServer(NULL), matchedLocation(NULL)
{
	serverIndex = -1;
	readBuffer = "";
	writeBuffer = "";
	httpVersion = "";
	keepAlive = true;
	shouldClose = false;
	lastActivity = std::time(NULL);
	req = HttpRequest();
	res = HttpResponse();
	cgiExecutable = "";
	cgiData = CgiContext();
}

Connection::Connection(const Connection& other) : CgiRequestHandler(), 
	fd(other.fd),
	serverIndex(other.serverIndex),
	matchedServer(other.matchedServer),
	matchedLocation(other.matchedLocation),
	readBuffer(other.readBuffer),
	writeBuffer(other.writeBuffer),
	httpVersion(other.httpVersion),
	keepAlive(other.keepAlive),
	shouldClose(other.shouldClose),
	lastActivity(other.lastActivity),
	req(other.req),
	res(other.res),
	state(other.state),
	cgiExecutable(other.cgiExecutable),
	cgiData(other.cgiData)
	{}

Connection& Connection::operator=(const Connection& other) {
	if (this != &other) {
		fd = other.fd;
		serverIndex = other.serverIndex;
		matchedServer = other.matchedServer;
		matchedLocation = other.matchedLocation;
		readBuffer = other.readBuffer;
		writeBuffer = other.writeBuffer;
		httpVersion = other.httpVersion;
		keepAlive = other.keepAlive;
		shouldClose = other.shouldClose;
		lastActivity = other.lastActivity;
		req = other.req;
		res = other.res;
		state = other.state;
		cgiExecutable = other.cgiExecutable;
		cgiData = other.cgiData;
	}
	return *this;
}

void	Connection::clear() {
	fd = -1;
	serverIndex = -1;
	readBuffer.clear();
	writeBuffer.clear();
	httpVersion.clear();
	keepAlive = true;
	shouldClose = false;
	lastActivity = std::time(NULL);
	req = HttpRequest();
	res = HttpResponse();
	cgiData = CgiContext();
	state = READING;
}

void	Connection::resetConnection() {
	writeBuffer.clear();
	httpVersion.clear();
	req = HttpRequest();
	res = HttpResponse();
	cgiData = CgiContext();
	state = READING;
}