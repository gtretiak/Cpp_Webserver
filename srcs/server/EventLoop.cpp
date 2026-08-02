/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:03:51 by nogioni-          #+#    #+#             */
/*   Updated: 2026/07/17 22:01:20 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"
#include "../http/Router.hpp"
#include "../config/globalConfig.hpp"
#include "../http/HttpException.hpp"
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
		{
			int		fd = _pollFds[i].fd;
			short	revents = _pollFds[i].revents;

			if (revents == 0)
				continue;

			if (revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (_cgifdToPollfd.count(fd)) {
					std::cout << "\n\n\nCGI POLLHUP FOR FD: "<< fd << "\n\n\n" << std::endl;
					continueCgi(fd);
					continue;
				}
				if (!isListenFd(fd)) {
					std::cout << "\n\n\nCLOSE CLIENT 1\n\n\n" << std::endl;
					closeClient(fd);
				}
				continue;
			}

			if ((revents & POLLIN) && isListenFd(fd))
				acceptClient(fd);
			else
			{
				if (revents & POLLIN) {
					if (_cgifdToPollfd.count(fd)) {//if the poll triggers the cgi fd, enter here
						std::cout << "\n\n\nCGI POLLIN\n\n\n" << std::endl;
						continueCgi(fd);
					}
					else //non cgi fd, enter here
						readClient(fd);
				}
				if (revents & POLLOUT) {
					std::cout << "EventLoop::run(): POLLOUT triggered for fd " << fd << std::endl;
					writeClient(fd);
				}
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

		Connection conn(clientFd);
		_connections[clientFd] = conn;

		struct pollfd pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		_pollFds.push_back(pfd);
		
		int	server_idx = _listenFdsMAP[listenFd];
		_listenFdsMAP.insert(std::make_pair(clientFd, server_idx));
		std::cout << "Client connected: fd " << clientFd << ", server index: " << server_idx << std::endl;
	}
}

void EventLoop::readClient(int clientFd)
{
	char		buffer[4096];
	Connection	&conn = _connections[clientFd];

	while (true)
	{
		ssize_t bytes = recv(clientFd, buffer, sizeof(buffer), 0);

		if (bytes > 0)
		{
			conn.readBuffer.append(buffer, bytes);
			conn.lastActivity = time(NULL);
		}
		else if (bytes == 0)
		{
			std::cout << "\n\n\nCLOSE CLIENT 2\n\n\n" << std::endl;
			closeClient(clientFd);
			return;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cout << "\n\n\nCLOSE CLIENT 3\n\n\n" << std::endl;
			closeClient(clientFd);
			return;
		}
	} //this block will read all the bytes available in the socket buffer, until it returns EAGAIN or EWOULDBLOCK, which means there are no more bytes to read at the moment

	if (!conn.readBuffer.empty() && conn.writeBuffer.empty())
	{
		std::cout << "----- RAW REQUEST from fd " << clientFd << " -----\n"
				<< conn.readBuffer
				<< "\n----- END REQUEST -----" << std::endl;

		try {
			HttpParser	parser;

			parser.parseRequest(conn.readBuffer, &conn.req);
			conn.readBuffer.clear();
			std::cout << "\n************** printRequest() **************" << std::endl;
			printRequest(conn.req);
			std::cout << "\n************** printRequest() (END) **************" << std::endl;

			if (conn.req.getUrl() == "/") { //VERSA0 PATH 
				std::ostringstream	resp;
				std::ostringstream	body;
				std::ifstream		file("www/html/index.html");
	
				body << file.rdbuf();
				std::string bodyContent = body.str();
				std::size_t bodySize = bodyContent.size();

				resp << "HTTP/1.1 200 OK\r\n"
					<< "Content-Type: text/html\r\n"
					<< "Content-Length: " << bodySize << "\r\n"
					<< "\r\n"
					<< bodyContent;
				
				conn.writeBuffer = resp.str();
				//this block is hardcoded for testing
			}
			/*else if (conn.req.getUrl() == "/favicon.ico") {
				std::ostringstream	resp;
				std::ostringstream	body;
				std::ifstream		file("www/html/favicon.ico", std::ios::binary);
	
				body << file.rdbuf();
				std::string bodyContent = body.str();
				std::size_t bodySize = bodyContent.size();

				resp << "HTTP/1.1 200 OK\r\n"
					<< "Content-Type: image/x-icon\r\n"
					<< "Content-Length: " << bodySize << "\r\n"
					<< "\r\n"
					<< bodyContent;
				
				conn.writeBuffer = resp.str();
			} //THIS BLOCK IS HARD-CODED FOR TESTING, REMOVE LATER*/
			else {
				Router	router;
				router.setConfig(_config);
				router.setConnEnv(conn);

				conn.state = RUNNING;
				
				// 0 for cgi 1 for static
				if (router.resolve(conn.req, conn.res) == 0) {
					_pollFds.push_back(conn.cgiData.pollFd);//register the cgi script output fd to the pollfd vector
					_cgifdToPollfd[conn.cgiData.outFd] = clientFd;

					writeRequestBodyToCgi(conn.req, conn.cgiData.inFd);
					close(conn.cgiData.inFd);
					conn.cgiData.inFd = -1;
				} else {
					size_t	bodySize = conn.res.getBody().size();
					if (bodySize > 0) {
						std::stringstream	ss;

						ss << bodySize;
						if (conn.res.hasHeader("Content-Length") == false)
							conn.res.setHeader("Content-Length", ss.str());
					}
					conn.writeBuffer = conn.res.toString();
				}
			}
			//printResponse(conn.res);
			conn.shouldClose = !conn.req.hasHeader("connection") || conn.req.getHeader("connection") != "keep-alive";
		}
		catch (const HttpException& e) {
			std::cout << e.what() << std::endl;
			std::cout << "strerror: " << strerror(errno) << std::endl;

			conn.state = CLOSING;
			handleHttpError(clientFd, e.code());
			conn.writeBuffer = conn.res.toString();
		}
		//conn.shouldClose = true;
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
		std::cout << "\n\n************** Sent " << sent << " bytes to client fd " << clientFd << std::endl;
		/*std::cout << "----- DEBUG RAW RESPONSE sent to fd " << clientFd << " -----\n"
				<< conn.writeBuffer.c_str()
				<< "$\n----- DEBUG END RESPONSE -----" << std::endl;*/
		conn.writeBuffer.erase(0, sent);
		conn.lastActivity = time(NULL);
	}
	else if (sent == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		std::cout << "\n\n\nCLOSE CLIENT 4\n\n\n" << std::endl;
		closeClient(clientFd);
		return;
	}

	if (conn.writeBuffer.empty())
	{
		if (conn.shouldClose || !conn.keepAlive)//
		{
			std::cout << "\n\n\nCLOSE CLIENT 5\n\n\n" << std::endl;
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

	std::cout << "Client closed: fd " << clientFd << std::endl;
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

	try {
		conn.setConfig(_config);
		conn.finalizeCgi(conn);
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
	

	if (conn.state == RUNNING) {
		_pollFds.push_back(conn.cgiData.pollFd);
		_cgifdToPollfd[conn.cgiData.outFd] = clientFd;
		writeRequestBodyToCgi(conn.req, conn.cgiData.inFd);
		close(conn.cgiData.inFd);
		conn.cgiData.inFd = -1;
		return ;
	}
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

	conn.res = HttpResponse();
	idx = matchConnToServerIndex(clientFd);
	std::cout << "EventLoop::handleHttpError(): clientFd = " << clientFd << ", errorCode = " << errorCode << ", server index = " << idx << std::endl;
	if (idx == -1) {//possible error, because clientFd do not match any server
		//error decide how to handle this case. maybe internal server error 500
		std::cout << "******** Error idx = -1 *********" << std::endl;
	}
	if (_config->servers[idx]._error_pages.empty()) {
		//no error pages configured use default
		// solve default by search e.code + html in root + error pages folder
	}
	else {
		std::map<int, std::string>::iterator	it;
		
		it = _config->servers[idx]._error_pages.find(errorCode);
		if (it != _config->servers[idx]._error_pages.end()) {
			//we find
			//second argument is the path
			std::string	filepath = _config->servers[idx]._root + it->second;
			conn.res.setStatus(errorCode);
			conn.res.generateErrorPageResponse(filepath.c_str());
			
			std::cout << "********** Error page generated from config file **********" << std::endl;
			printResponse(conn.res);
			std::cout << "********** Error page generated from config file (END) **********" << std::endl;
		}
	}
}