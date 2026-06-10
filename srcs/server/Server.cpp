/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:25:16 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/10 21:26:50 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _socket(),
				   _eventLoop()
{
}

Server::~Server()
{
}

void Server::setup(int port)
{
	_socket.create(port);
	_eventLoop.addListenFd(_socket.getFd());
}

void Server::run()
{
	_eventLoop.run();
}

void Server::stop()
{
	_eventLoop.stop();
}