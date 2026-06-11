/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 20:19:23 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/10 21:02:54 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

#include <stdexcept>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

Socket::Socket() : _fd(-1) {}

Socket::~Socket() {
	closeSocket();
}

void	Socket::create(int port)
{
	int	option;
	struct	sockaddr_in	address; // represents ipv4 address

	// domain: AF_INET - IPv4 Internet protocols
	// type: SOCK_STREAM - connection-base byte streams
	// protocol: 0 - default protocol for this kind AF_INET + SOCK_STREAM = TCP
	_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (_fd == -1)
		throw std::runtime_error("socket() failed");
	
	option = 1;

	//_fd: socket that will be configured
	//SOL_SOCKET:: level of configuration
	//SO_REUSEADDR: wich option I want to modify -> allows to reuse the port rapidly
	//&option: the new value I want to put in this option
	//sizeof: the size of the value passed by optval
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
	{
		closeSocket();
			throw std::runtime_error("setsockopt() failed");
	}
	
	// set mem to zero
	std::memset(&address, 0, sizeof(address));

	//this address is IPV4
	address.sin_family = AF_INET;

	//s_addr: IP in binary
	//htonl(): converts the value to network byte order
	//INADDR_ANY: 0.0.0.0 - accept connections on any interface
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	// defines wich port the server will listen from
	address.sin_port = htons(port);
	
	// bind expects a generic pointer so we do the cast
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
	{
		closeSocket();
		throw std::runtime_error("bind() failed");
	}

	//passive mode: the server is now capable of accepting connections
	//SOMAXCONN: maximum of pendind connections
	//waiting for accept()
	if (listen(_fd, SOMAXCONN) == -1)
	{
		closeSocket();
		throw std::runtime_error("listen() failed");
	}

	//sets the socket in non-blocking mode -> useful to poll()
	setNonBlocking(_fd);
}
int	Socket::getFd() const
{
	return (_fd);
}

void	Socket::closeSocket()
{
	if (_fd != -1)
	{
		close(_fd);
		_fd = -1;
	}
}

void	Socket::setNonBlocking(int fd)
{
	int flags;

	//F_GETFL: get the actual flags
	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		closeSocket();
		throw std::runtime_error("fcntl(F_GETFL) failed");
	}

	//F_SETFL: take the actual flags and adds O_NONBLOCK
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		closeSocket();
		throw std::runtime_error("fcntl(F_SETFL) failed");
	}
}