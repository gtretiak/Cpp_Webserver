/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:03:51 by nogioni-          #+#    #+#             */
/*   Updated: 2026/08/27 01:31:32 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"
#include "../http/Router.hpp"
#include "../config/globalConfig.hpp"
#include "../http/HttpException.hpp"
#include "../http/HttpUtils.hpp"
#include "Connection.hpp"
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sstream>
#include <fstream>

volatile sig_atomic_t	g_shutdown = 0;

EventLoop::EventLoop() : _pollFds(),
						_listenFds(),
						_connections(),
						_config(NULL),
						_running(false) {}

EventLoop::EventLoop(const EventLoop& other) : _pollFds(other._pollFds),
												_listenFds(other._listenFds),
												_connections(other._connections),
												_config(other._config),
												_running(other._running) {}

EventLoop& EventLoop::operator=(const EventLoop& other) {
	if (this != &other)
	{
		_pollFds = other._pollFds;
		_listenFds = other._listenFds;
		_connections = other._connections;
		_config = other._config;
		_running = other._running;
	}
	return *this;
}

EventLoop::~EventLoop()
{
	for (size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd != -1)
			close(_pollFds[i].fd);
	}
}

void	signalHandler(int signal) {
	(void)signal;
	g_shutdown = 1;
}

void EventLoop::addListenFd(int fd, int server_idx)
{
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_pollFds.push_back(pfd);
	_listenFds.push_back(fd);
	_listenFdsMAP.insert(std::make_pair(fd, server_idx));
}

void	EventLoop::abortCgiInput(int fd) {
	int			clientFd = _cgiInfdToPollfd[fd];
	Connection	&conn = _connections[clientFd];

	removeFdFromPollFds(fd);
	_cgiInfdToPollfd.erase(fd);
	close(fd);
	conn.cgiData.inFd = -1;
	(void)conn;
}

void	EventLoop::writeCgiInput(int fd) {
	int					clientFd = _cgiInfdToPollfd[fd];
	Connection			&conn = _connections[clientFd];
	const std::string	&requestBody = conn.req.getBody();

	size_t	remaining;
	ssize_t	sent;

	remaining = requestBody.size() - conn.cgiData.bodyBytesSent;
	sent = write(fd, requestBody.c_str() + conn.cgiData.bodyBytesSent, remaining);
	if (sent > 0)
		conn.cgiData.bodyBytesSent += static_cast<size_t>(sent);
	else if (sent == -1) {
		abortCgiInput(fd);
		return ;
	}
	if (conn.cgiData.bodyBytesSent >= requestBody.size()) {
		removeFdFromPollFds(fd);
		_cgiInfdToPollfd.erase(fd);
		close(fd);
		conn.cgiData.inFd = -1;
	}
}

void EventLoop::run(globalConfig& config)
{
	_running = true;
	_config = &config;

	while (_running)
	{
		if (g_shutdown)
		{
			std::cout << "\nShutting down server..." << std::endl;
			break;
		}
		int ready = poll(&_pollFds[0], _pollFds.size(), 1000);

		if (ready == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll() failed");
		}

		//iterates backards bacause closeClient() might remove elements from the array
		for (int i = static_cast<int>(_pollFds.size()) - 1; i >= 0; --i)
		{ //wrap a try block here?
			try {
				int		fd = _pollFds[i].fd;
				short	revents = _pollFds[i].revents;

				if (revents == 0)
					continue;

				if (revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					if (_cgifdToPollfd.count(fd)) { //cgi out fd trigged, redirections case
						std::cout << "\n\t(log) (1)POLLERR | POLLHUP | POLLNVAL: trigged for cgi out fd:"<< fd << "\n" << std::endl;
						continueCgi(fd);
						continue;
					}
					if (_cgiInfdToPollfd.count(fd)) { //cgi in fd trigged, redirections case
						std::cout << "\n\t(log) (2)POLLERR | POLLHUP | POLLNVAL: trigged for cgi in fd: "<< fd << "\n" << std::endl;
						abortCgiInput(fd);
						continue;
					}
					if (!isListenFd(fd)) {
						std::cout << "\n\t(log) EventLoop::run(): closeClient() trigged for fd: "<< fd << "\n" << std::endl;
						closeClient(fd);
					}
					continue;
				}

				if ((revents & POLLIN) && isListenFd(fd))
					acceptClient(fd);
				else
				{
					if (revents & POLLIN) {
						if (_cgifdToPollfd.count(fd))//cgi out fd trigged
							continueCgi(fd);
						else //non cgi fd trigged
							readClient(fd);
					}
					if (revents & POLLOUT) {
						if (_cgiInfdToPollfd.count(fd)) //cgi in fd trigged
							writeCgiInput(fd);
						else //non cgi fd trigged
							writeClient(fd);
					}
				}
			} catch (const std::exception& e) {
				std::cerr << "Error in EventLoop::run(): " << e.what() << std::endl;
				std::cerr << "strerror: " << strerror(errno) << std::endl;
			}
		}
	}
}

void EventLoop::stop()
{
	_running = false;
}

bool EventLoop::isListenFd(int fd) const
{
	for (size_t i = 0; i < _listenFds.size(); ++i)
	{
		if (_listenFds[i] == fd)
			return true;
	}
	return false;
}

void EventLoop::acceptClient(int listenFd)
{
	while (true)
	{
		int clientFd = accept(listenFd, NULL, NULL);

		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			throw std::runtime_error("accept() failed");
		}

		int flags = fcntl(clientFd, F_GETFL, 0);
		if (flags == -1)
		{
			close(clientFd);
			throw std::runtime_error("fcntl(F_GETFL) failed on client");
		}

		if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			close(clientFd);
			throw std::runtime_error("fcntl(F_SETFL) failed on client");
		}

		int server_idx = _listenFdsMAP[listenFd];

		Connection conn(clientFd);
		conn.serverIndex = server_idx;
		_connections[clientFd] = conn;

		struct pollfd pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		_pollFds.push_back(pfd);
		_listenFdsMAP.insert(std::make_pair(clientFd, server_idx));
		std::cout << "Client connected: fd " << clientFd << ", server index: " << server_idx << std::endl;
	}
}

bool EventLoop::locationMatches(const std::string &locationPath, const std::string &requestPath) const
{
	if (locationPath.empty())
		return false;
	if (locationPath == "/")
		return true;
	if (requestPath.compare(0, locationPath.size(), locationPath) != 0)
		return false;
	return true;
}

locationConfig *EventLoop::findBestLocation(serverConfig &server, const std::string &path)
{
	locationConfig *best = NULL;
	size_t bestLen = 0;

	for (size_t i = 0; i < server._locations.size(); ++i)
	{
		std::string locationPath = server._locations[i]._path;

		if (locationMatches(locationPath, path) && locationPath.size() > bestLen)
		{
			best = &server._locations[i];
			bestLen = locationPath.size();
		}
	}
	return best;
}

void EventLoop::readClient(int clientFd)
{
	char		buffer[65536];
	Connection	&conn = _connections[clientFd];


	//1* Read once. No while loop. No errno checks.
	ssize_t bytes = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytes > 0)
	{
		conn.readBuffer.append(buffer, bytes);
		conn.lastActivity = time(NULL);
	}
	else if (bytes == 0)
	{
		//std::cout << "\n\n\nCLOSE CLIENT 2\n\n\n" << std::endl;
		closeClient(clientFd);
		return;
	}
	else
	{
		std::cout << "\n\n\ncloseClient at readClient (recv Error)\n\n\n" << std::endl;
		closeClient(clientFd);
		return;
	}

	//2* check the request completeness and parse it if complete. If not, wait for more data.
	bool	isRequestComplete = validateRawBufferRequest(conn);

	if (isRequestComplete && conn.writeBuffer.empty())
	{
		try
		{
			HttpParser parser;

			if (_config && conn.serverIndex >= 0 && static_cast<size_t>(conn.serverIndex) < _config->servers.size() && _config->servers[conn.serverIndex]._has_client_max_body_size)
			{
				parser.setMaxBodySize(_config->servers[conn.serverIndex]._client_max_body_size);
			}

			if (!parser.isRequestComplete(conn.readBuffer))
				return;

			//DEBUG , remove this block, only used to check raw request received from client, last request only
			/*int fd = open("tester_logs/log_1.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd != -1) {
				write(fd, "\t----- RAW REQUEST ------", 26);
				write(fd, conn.readBuffer.c_str(), conn.readBuffer.size());
				write(fd, "\n\t----- END REQUEST -----\n", 27);
				close(fd);
			}*/

			size_t consumed = parser.parseRequest(conn.readBuffer, &conn.req);
			conn.readBuffer.erase(0, consumed);

			std::cout << "\n************** printRequest() **************" << std::endl;
			//printRequest(conn.req);
			std::cout << "\n************** printRequest() (END) **************" << std::endl;

			Router router;
			router.setConfig(_config);
			router.setConnEnv(conn);

			conn.state = RUNNING;

			if (router.resolve(conn.req, conn.res) == 0)
			{
				_pollFds.push_back(conn.cgiData.pollFd);
				_cgifdToPollfd[conn.cgiData.outFd] = clientFd;

				if (!conn.req.getBody().empty()) {
					conn.cgiData.bodyBytesSent = 0;
					conn.cgiData.inPollFd.fd = conn.cgiData.inFd;
					conn.cgiData.inPollFd.events = POLLOUT;
					conn.cgiData.inPollFd.revents = 0;
					_pollFds.push_back(conn.cgiData.inPollFd);
					_cgiInfdToPollfd[conn.cgiData.inFd] = clientFd;
				} else {
					close(conn.cgiData.inFd); // sem body, sem write, o close não precisa de poll
					conn.cgiData.inFd = -1;
				}
			}
			else
			{
				size_t bodySize = conn.res.getBody().size();
				if (bodySize > 0)
				{
					std::stringstream ss;

					ss << bodySize;
					if (conn.res.hasHeader("Content-Length") == false)
						conn.res.setHeader("Content-Length", ss.str());
				}
				conn.writeBuffer = conn.res.toString();
			}

			conn.shouldClose = false;

			if (!conn.req.hasHeader("connection") || conn.req.getHeader("connection") != "keep-alive")
				conn.shouldClose = true;

			if (conn.res.hasHeader("connection") && conn.res.getHeader("connection") == "close")
				conn.shouldClose = true;
		}
		catch (const HttpException &e)
		{
			std::cout << e.what() << std::endl;
			std::cout << "strerror: " << strerror(errno) << std::endl;

			conn.state = CLOSING;
			handleHttpError(clientFd, e.code());
			conn.writeBuffer = conn.res.toString();
		}

		updateClientEvents(clientFd);
	}
}

void EventLoop::writeClient(int clientFd)
{
	Connection &conn = _connections[clientFd];

	if (conn.writeBuffer.empty())
	{
		updateClientEvents(clientFd);
		return;
	}

	ssize_t sent = send(clientFd, conn.writeBuffer.c_str(), conn.writeBuffer.size(), 0);

	if (sent > 0)
	{
		//std::cout << "\n\n************** Sent " << sent << " bytes to client fd " << clientFd << std::endl;
		/*std::cout << "----- DEBUG RAW RESPONSE sent to fd " << clientFd << " -----\n"
				<< conn.writeBuffer.c_str()
				<< "$\n----- DEBUG END RESPONSE -----" << std::endl;*/
		conn.writeBuffer.erase(0, sent);
		conn.lastActivity = time(NULL);
	}
	else if (sent == -1)
	{
		//no need to check for EAGAIN or EWOULDBLOCK, because POLLOUT guaratees the socket availability to write, so if send() returns -1, it is a real error
		//std::cout << "\n\n\nCLOSE CLIENT 4\n\n\n" << std::endl;
		closeClient(clientFd);
		return;
	}

	if (conn.writeBuffer.empty())
	{
		if (conn.shouldClose || !conn.keepAlive)//
		{
			//std::cout << "\n\n\nCLOSE CLIENT 5\n\n\n" << std::endl;
			closeClient(clientFd);
		}
		else {
			conn.resetConnection();
			updateClientEvents(clientFd);
		}
	}
}

void EventLoop::closeClient(int clientFd)
{
	close(clientFd);
	_connections.erase(clientFd);

	for (std::vector<struct pollfd>::iterator it = _pollFds.begin();
		 it != _pollFds.end(); ++it)
	{
		if (it->fd == clientFd)
		{
			_pollFds.erase(it);
			_listenFdsMAP.erase(clientFd);
			break;
		}
	}

	//std::cout << "Client closed: fd " << clientFd << std::endl;
}

void EventLoop::updateClientEvents(int clientFd)
{
	for (size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd == clientFd)
		{
			if (!_connections[clientFd].writeBuffer.empty())
				_pollFds[i].events = POLLOUT;
			else
				_pollFds[i].events = POLLIN;
			_pollFds[i].revents = 0;
			return;
		}
	}
}

void	EventLoop::continueCgi( int pipeFd ) {
	int			clientFd = _cgifdToPollfd[pipeFd];
	Connection	&conn = _connections[clientFd];

	char	buf[4096];
	ssize_t	bytesRead = read(pipeFd, buf, sizeof(buf));
	
	if (bytesRead > 0) {
		conn.cgiData.outputBuffer.append(buf, bytesRead);
		return ;
	}
	removeFdFromPollFds(pipeFd);
	_cgifdToPollfd.erase(pipeFd);
	close(pipeFd);

	int status;
	//1) write request body to cgi, is called at read client
	//2) READ CGI OUTPUT, is called here, accumulate bytes from cgi script stdout
	//3) waitpid, is called here, to avoid blocking the main loop
	
	pid_t	deadPid = conn.cgiData.pid;
	conn.cgiData.pid = -1;
	conn.cgiData.outFd = -1;
	waitpid(deadPid, &status, WNOHANG);

	int	pendingInFd = conn.cgiData.inFd;

	try {
		conn.setConfig(_config);
		conn.finalizeCgi(conn);//adapt finalizeCgi for streaming cgi data
	}
	catch (const HttpException& e) {
		std::cout << e.what() << std::endl;
		std::cout << "strerror: " << strerror(errno) << std::endl;

		handleHttpError(clientFd, e.code());
		conn.state = CLOSING;
		conn.writeBuffer = conn.res.toString();
		updateClientEvents(clientFd);
		return ;
	}

	if (pendingInFd != -1 && _cgiInfdToPollfd.count(pendingInFd)) {
		removeFdFromPollFds(pendingInFd);
		_cgiInfdToPollfd.erase(pendingInFd);
		close(pendingInFd);
		conn.cgiData.inFd = -1;
	}

	if (conn.state == RUNNING) {
		_pollFds.push_back(conn.cgiData.pollFd);
		_cgifdToPollfd[conn.cgiData.outFd] = clientFd;
		
		if (!conn.req.getBody().empty()) {
			conn.cgiData.bodyBytesSent = 0;
			conn.cgiData.inPollFd.fd = conn.cgiData.inFd;
			conn.cgiData.inPollFd.events = POLLOUT;
			conn.cgiData.inPollFd.revents = 0;
			_pollFds.push_back(conn.cgiData.inPollFd);
			_cgiInfdToPollfd[conn.cgiData.inFd] = clientFd;
		} else {
			close(conn.cgiData.inFd);   // sem body, sem write, o close não precisa de poll
			conn.cgiData.inFd = -1;
		}
		return ;
	}
	//THIS BLOCK ABOVE MAY NEED ADAPTATION. if we send data in chunks
	size_t	bodySize = conn.res.getBody().size();
	if (bodySize > 0) {
		std::stringstream	ss;

		ss << bodySize;
		if (conn.res.hasHeader("Content-Length") == false)
			conn.res.setHeader("Content-Length", ss.str());
	}
	conn.writeBuffer = conn.res.toString();
	updateClientEvents(clientFd);
}

void	EventLoop::removeFdFromPollFds( int fd ) {
	std::vector<struct pollfd>::iterator it = _pollFds.begin();

	for (; it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			break;
		}
	}
}

int	EventLoop::matchConnToServerIndex( int clientFd ) {
	std::map<int, int>::iterator	it;

	it = _listenFdsMAP.find(clientFd);

	/*for (std::map<int, int>::iterator jt = _listenFdsMAP.begin(); jt != _listenFdsMAP.end(); ++jt) {
		std::cout << "EventLoop::matchConnToServerIndex(): listenFdMAP = " << jt->first << ", server index = " << jt->second << std::endl;
	}*/
	if (it != _listenFdsMAP.end()) {
		return it->second;
	}
	return -1;
}

void	EventLoop::handleHttpError( int clientFd, int errorCode ) {
	int			idx;
	Connection	&conn = _connections[clientFd];
	std::string	errorPagePath;

	conn.res = HttpResponse();
	conn.res.setStatus(errorCode);
	conn.res.setHeader("connection", "close");
	conn.shouldClose = true;

	idx = matchConnToServerIndex(clientFd);
	std::cout << "EventLoop::handleHttpError(): clientFd = " << clientFd << ", errorCode = " << errorCode << ", server index = " << idx << std::endl;
	if (idx == -1) {
		//error, because clientFd do not match any server
		//generate internal server error 500
		//means that we not have any root or error_page directive to generate the error page
		std::cout << "******** Error(handleHttpError): client does not match any server *********" << std::endl;
		conn.res.setStatus(500);
		conn.res.generateErrorPageResponse(NULL, 500);
		return ;
	}
	if (_config->servers[idx]._error_pages.empty()) {
		std::cout << "********** (handleHttpError): server does not have any error_page directive configured **********" << std::endl;
		conn.res.setStatus(errorCode);
		conn.res.generateErrorPageResponse(NULL, errorCode);
	}
	else {
		std::map<int, std::string>::iterator	it;

		it = _config->servers[idx]._error_pages.find(errorCode);
		if (it != _config->servers[idx]._error_pages.end()) {//find error_page directive
			std::cout << "********** Error page generated from config file **********" << std::endl;
			//it->second is the path to error page
			std::string	filepath = _config->servers[idx]._root + it->second;
			conn.res.setStatus(errorCode);
			conn.res.generateErrorPageResponse(filepath.c_str(), errorCode);
			
			std::cout << "********** Error page generated from config file **********" << std::endl;
			printResponse(conn.res);
			std::cout << "********** Error page generated from config file (END) **********" << std::endl;
		}
		else { //do not find the error code among the error_pages configured
			conn.res.setStatus(errorCode);
			conn.res.generateErrorPageResponse(NULL, errorCode);
		}
	}
}

/// @brief build logic to check if raw buffer of request is complete.
/// @brief If not, return and wait for more data.
/// @param buffer Raw buffer of request readed with recv() from client socket
/// @param conn Connection object
/// @return true if request is complete, false if not
bool	EventLoop::validateRawBufferRequest( Connection& conn ) {
	size_t		headerEndPos;
	std::string	headers;

	headerEndPos = conn.readBuffer.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
		return (false);
	headers = toLower(conn.readBuffer.substr(0, headerEndPos));

	if (headers.compare(0, 4, "get ") == 0 || headers.compare(0, 7, "delete ") == 0)
		return (true);
	else if (headers.compare(0, 5, "post ") == 0)
	{
		size_t	contentLengthPos = headers.find("content-length:");
		if (contentLengthPos != std::string::npos)
		{
			size_t	valueStart = contentLengthPos + 15;
			size_t	lineEnd = headers.find("\r\n", valueStart);
			
			std::string	lengthStr = headers.substr(valueStart, lineEnd - valueStart);
			size_t		expectedLength = static_cast<size_t>(std::atoi(lengthStr.c_str()));
			
			size_t bodyStartPos = headerEndPos + 4;
			size_t currentBodySize = conn.readBuffer.size() - bodyStartPos;
			return currentBodySize >= expectedLength;
		}
		return (true);
	}
	return (true);//unkwon methiod
}