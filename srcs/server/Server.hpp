/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:21:10 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/22 23:29:37 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include "Socket.hpp"

class	EventLoop;
struct	globalConfig;

class	Server
{
	private:
		std::vector<Socket>	_sockets;
		Socket		_socket; //represents the listen socket -> the open door
		EventLoop*	_eventLoop; //represent the loop -> poll()
	public:
		Server();
		Server( EventLoop& eventLoop );
		Server( const Server& other );
		Server& operator=(const Server& other );
		~Server();

		// creates the socket on the chosen door and 
		// ads the fd on the event loop
		void	setup(int port);
		void	setup( Listen target );

		//starts the loop
		void	run(globalConfig& config);
		//stops the loop
		void	stop();
};

#endif