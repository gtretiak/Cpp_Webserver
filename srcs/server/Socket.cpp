/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 20:19:23 by nogioni-          #+#    #+#             */
/*   Updated: 2026/08/13 18:56:40 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cerrno>
#include "../config/locationConfig.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>

Socket::Socket() : _fd(-1) {}

Socket::~Socket() {
	closeSocket();
}

void	Socket::setPortOnlyConn( Listen target ) {
	struct	sockaddr_in	address;
	int		opt;

	_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC , 0);
	if (_fd < 0)
		throw std::runtime_error("Error: socket(): opening socket failed");
	opt = 1;

	//MAYBE ADD SO_KEEPALIVE, depends on directive and server config
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		closeSocket();
		throw std::runtime_error("Error: setsockopt(): failed");
	}

	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(target.port);

	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&address),
			sizeof(address)) < 0) {
		closeSocket();
		std::cout << strerror(errno) << " : " << errno << std::endl;
		throw std::runtime_error("Error: bind(): binding failed");
	}//after this the socket is ready to receive data so we call listen.

	if (listen(_fd, SOMAXCONN) < 0) {
		closeSocket();
		throw std::runtime_error("Error: listen(): failed");
	}
}

/// @brief Setter for IP + PORT connection branch,
/// @brief Documentation: https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
/// @param target 
void	Socket::setIpHostPortConn( Listen target, Listen::listenType type ) {
	struct	addrinfo	hints;
	struct	addrinfo	*res = NULL;
	std::ostringstream	oss;
	std::string			portstr;
	int					rc;
	int					fd = -1;
	int					opt = 1;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (type == Listen::IP_PORT)
		hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
	else
		hints.ai_flags = 0;
	hints.ai_protocol = 0;

	oss << target.port;
	portstr = oss.str();
	//node specifies numerical network address or network hostname, 
	//IF AI_NUMERICHOST is set, the node must be numerical network address, useful for ip + port branch
	//service sets the port
	//if AI_NUMERICSERV is set the service must point to a string containing a numeric port number
	if (target.addr == "*")
		target.addr = "0.0.0.0";
	rc = getaddrinfo(target.addr.c_str(), portstr.c_str(), &hints, &res);
	if (rc != 0) {
		throw std::runtime_error(std::string("Error: setIpHostPortConn: getaddrinfo failed") + gai_strerror(rc));
	}

	for ( addrinfo *tmp = res; tmp != NULL; tmp = tmp->ai_next ) {
		fd = socket(tmp->ai_family, tmp->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC, tmp->ai_protocol);
		if (fd == -1)
			continue ;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			closeSocket(fd);
			continue ;
		}
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
			// if you require SO_REUSEPORT, treat as error; otherwise you can ignore
			// close(fd); fd = -1; continue;
		}
		setNonBlocking(fd);
		if (bind(fd, tmp->ai_addr, tmp->ai_addrlen) == 0) {
			if (listen(fd, SOMAXCONN) == 0) {
				_fd = fd;
				break;
			}
		}
		else {
			std::cout << strerror(errno) << " : " << errno << std::endl;
			std::cout << "bind failed for address: " << target.addr << " port: " << target.port << std::endl;
		}
		closeSocket(fd);
	}
	freeaddrinfo(res);
	if (fd == -1)
		throw std::runtime_error("Error: setIpHostPortConn(): bind/listen failed for any resolved address");
}

void	Socket::setUnixConn( Listen target ) {
	struct sockaddr_un	address;
	socklen_t			servlen;
	int					namelen;

	_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (_fd < 0) {
		std::cout << strerror(errno) << std::endl;
		throw	std::runtime_error("Error: setUnixConn(): opening socket failed");
	}
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	namelen = std::min(target.addr.size(), sizeof(address.sun_path) - 1);
	address.sun_path[0] = '\0';
	std::memcpy(address.sun_path + 1, target.addr.data(), namelen);
	servlen = static_cast<socklen_t>(offsetof(struct sockaddr_un, sun_path) + 1 + namelen);

	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&address), servlen) < 0) {
		std::cout << "_fd: " << _fd
		<< "\ntarget addr: " << target.addr
		<< "\ntarget port: " << target.port
		<< "\ntarget type: " << target.type
		<< std::endl;
		closeSocket();
		std::cout << strerror(errno) << std::endl;
		throw std::runtime_error("Error: setUnixConn(): biding failed");
	}
	if (listen(_fd, SOMAXCONN) < 0) {
		closeSocket();
		std::cout << strerror(errno) << std::endl;
		throw std::runtime_error("Error: setUnixConn(): listen() failed");
	}
}

//htons / ntohs for 16-bit values (ports)
//htonl / ntohl for 32-bit values (ipv4 address)

void	Socket::create(Listen target) {
	switch (target.type) {
		case Listen::PORT:
			setPortOnlyConn(target);
			break;
		case Listen::IP_PORT:
			setIpHostPortConn(target, target.type);
			break;
		case Listen::HOST_PORT:
			setIpHostPortConn(target, target.type);
			break;
		case Listen::UNIX_SOCK:
			setUnixConn(target);
			break;
		default:
			std::cout << "Failed to create socket: missing type" << std::endl;
			break;
	}
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

void	Socket::closeSocket(int fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

void	Socket::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		closeSocket();
		throw std::runtime_error("fcntl(F_SETFL) failed");
	}
}