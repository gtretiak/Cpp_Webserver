/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 21:03:53 by nogioni-          #+#    #+#             */
/*   Updated: 2026/08/06 17:08:21 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "../http/HttpParser.hpp"
#include "../cgi/CgiRequestHandler.hpp"
#include <vector>
#include <map>
#include <poll.h>
#include <signal.h>
#include "../config/serverConfig.hpp"
#include "../config/locationConfig.hpp"

extern volatile sig_atomic_t	g_shutdown;

void	signalHandler(int signal);

struct	globalConfig;
class	Connection;
class Router;

class	EventLoop
{
	private:
		std::vector<struct pollfd>		_pollFds;		//keeps all the fds monitorized by poll()
		std::map<int, int>				_listenFdsMAP;
		std::vector<int>				_listenFds;		//keeps only the server sockets
		std::map<int, Connection>		_connections;	//list of all clients connected at the time 
						//(Connection: complete status of the client)
		std::map<int, int>				_cgifdToPollfd; //maps the cgi script fd to the pollfd index, so we can find it in the _pollFds vector
		globalConfig*					_config;		//pointer to the global config, used to route requests
		Router*							_router;		//used to route requests to the correct handler
		bool							_running; //controls the main loop

	public:
		EventLoop();
		EventLoop(const EventLoop& other);
		EventLoop& operator=(const EventLoop& other);
		~EventLoop();

		void	addListenFd(int fd, int server_idx); //adds a server socket to the loop
		void	run(globalConfig& config);	//main function of the server
		void	stop();	//ends loop

	// can't be called directly by who is using the class 
	// only make sense inside the run() flow
	// avoid that the user puts the object in an invalid status
	private:
		bool	isListenFd(int fd) const; //does this fd belongs to a server socket?
		void	acceptClient(int listenFd); //receives a new connection
		void	readClient(int clientFd); //reads client data
		void	writeClient(int clientFd); //sends answer
		void	closeClient(int clientFd); //closes and cleans a client
		void	updateClientEvents(int clientFd); //it changes which events the poll() function should observe
		void	removeFdFromPollFds(int fd); //removes a fd from the pollfd vector, used when closing a client or cgi script
		void	continueCgi(int pipeFd);

		int		matchConnToServerIndex(int clientFd);
		void	handleHttpError(int clientFd, int errorCode);

		locationConfig *findBestLocation(serverConfig &server, const std::string &path);
		bool locationMatches(const std::string &locationPath, const std::string &requestPath) const;
		size_t getMaxBodySizeForRequest(const std::string &readBuffer, int serverIndex);
};

#endif