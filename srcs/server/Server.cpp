/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:25:16 by nogioni-          #+#    #+#             */
/*   Updated: 2026/08/12 21:51:05 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "../config/locationConfig.hpp"
#include "../config/globalConfig.hpp"
#include "EventLoop.hpp"
#include <errno.h>

Server::Server() :	_socket(), _eventLoop()
{}
Server::Server( EventLoop& eventLoop ) : _socket(), _eventLoop(&eventLoop)
{}

Server::Server( const Server& other ) : _socket(other._socket),
	_eventLoop(other._eventLoop) {
}

Server&	Server::operator=(const Server& other ) {
	if (this != &other) {
		_socket = other._socket;
		_eventLoop = other._eventLoop;
	}
	return *this;
}

Server::~Server()
{
}

void Server::setup(int port, int server_idx)
{
	_socket.create(port);
	_eventLoop->addListenFd(_socket.getFd(), server_idx);
}

void	Server::setup( Listen target, int server_idx ) {
	//maybe but this function to create a fd and add to the std::vector of sockets
	//this way we can have multiple listen sockets
	_socket.create(target);
	_eventLoop->addListenFd(_socket.getFd(), server_idx);
}

void Server::run(globalConfig& config)
{
	try {
		_eventLoop->run(config);
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal Error (Server::run) Server.cpp:58 : " << e.what() << std::endl;
		std::cout << strerror(errno) << std::endl;
	}
}

void Server::stop()
{
	_eventLoop->stop();
}