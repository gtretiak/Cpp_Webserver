/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:21:10 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/10 21:25:12 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"
#include "EventLoop.hpp"


class	Server
{
	private:
		Socket	_socket; //represents the listen socket -> the open door
		EventLoop	_eventLoop; //represent the loop -> poll()
	public:
		Server();
		~Server();

		// creates the socket on the chosen door and 
		// ads the fd on the event loop
		void	setup(int port);

		//starts the loop
		void	run();
		//stops the loop
		void	stop();
};

#endif