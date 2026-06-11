/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:03:51 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/10 21:20:42 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventLoop.hpp"

#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

EventLoop::EventLoop() : _pollFds(),
						_listenFds(),
						_connections(),
						_running(false)
{
}

EventLoop::~EventLoop()
{
	for (size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd != -1)
			close(_pollFds[i].fd);
	}
}

void EventLoop::addListenFd(int fd)
{
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_pollFds.push_back(pfd);
	_listenFds.push_back(fd);
}

void EventLoop::run()
{
	_running = true;

	while (_running)
	{
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
			int fd = _pollFds[i].fd;
			short revents = _pollFds[i].revents;

			if (revents == 0)
				continue;

			if (revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (!isListenFd(fd))
					closeClient(fd);
				continue;
			}

			if ((revents & POLLIN) && isListenFd(fd))
				acceptClient(fd);
			else
			{
				if (revents & POLLIN)
					readClient(fd);
				if (revents & POLLOUT)
					writeClient(fd);
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

		std::cout << "Client connected: fd " << clientFd << std::endl;
	}
}

void EventLoop::readClient(int clientFd)
{
	char buffer[4096];
	Connection &conn = _connections[clientFd];

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
			closeClient(clientFd);
			return;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			closeClient(clientFd);
			return;
		}
	}

	if (!conn.readBuffer.empty() && conn.writeBuffer.empty())
	{
		conn.writeBuffer =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 12\r\n"
			"Connection: close\r\n"
			"\r\n"
			"Hello world\n";

		conn.shouldClose = true;
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
		conn.writeBuffer.erase(0, sent);
		conn.lastActivity = time(NULL);
	}
	else if (sent == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		closeClient(clientFd);
		return;
	}

	if (conn.writeBuffer.empty())
	{
		if (conn.shouldClose || !conn.keepAlive)
			closeClient(clientFd);
		else
			updateClientEvents(clientFd);
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