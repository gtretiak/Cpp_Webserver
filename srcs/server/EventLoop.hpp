/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:03:53 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/16 22:51:20 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "Connection.hpp"

#include <vector>
#include <map>
#include <poll.h>

class	EventLoop
{
	private:
		std::vector<struct pollfd>		_pollFds;		//keeps all the fds monitorized by poll()
		std::vector<int>				_listenFds;		//keeps only the server sockets
		std::map<int, Connection>		_connections;	//list of all clients connected at the time 
						//(Connection: complete status of the client)

		bool	_running; //controls the main loop

	public:
		EventLoop();
		~EventLoop();

		void	addListenFd(int fd); //adds a server socket to the loop
		void	run();	//main function of the server
		void	stop();	//ends loop

	// can't be called directly by who is using the class 
	// only make sense inside the run() flow
	// avoid that the user puts the object in an invalid status
	private:
		bool isListenFd(int fd) const; //does this fd belongs to a server socket?
		void acceptClient(int listenFd); //receives a new connection
		void readClient(int clientFd); //reads client data
		void writeClient(int clientFd); //sends answer
		void closeClient(int clientFd); //closes and cleans a client
		void updateClientEvents(int clientFd); //it changes which events the poll() function should observe
};

#endif