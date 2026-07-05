/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:25:16 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/22 23:30:56 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "../config/locationConfig.hpp"
#include "../config/globalConfig.hpp"
#include "EventLoop.hpp"

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

void Server::setup(int port)
{
	_socket.create(port);
	_eventLoop->addListenFd(_socket.getFd());
}

void	Server::setup( Listen target ) {
	//maybe but this function to create a fd and add to the std::vector of sockets
	//this way we can have multiple listen sockets
	_socket.create(target);
	_eventLoop->addListenFd(_socket.getFd());
}

void Server::run(globalConfig& config)
{
	_eventLoop->run(config);
}

void Server::stop()
{
	_eventLoop->stop();
}