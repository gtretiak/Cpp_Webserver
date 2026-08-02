/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_testing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 21:38:29 by dopereir          #+#    #+#             */
/*   Updated: 2026/07/13 11:03:36 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/globalConfig.hpp"
#include "config/configParser.hpp"
#include "server/EventLoop.hpp"
#include "server/Server.hpp"
#include <exception>

int	main(int ac, char** av)
{
	globalConfig	config;
	EventLoop		eventLoop;
	Server			webserv(eventLoop);

	signal(SIGINT, signalHandler); //ctrl + c
	signal(SIGQUIT, signalHandler);//ctrl + '\'
	signal(SIGTERM, signalHandler);//kill command
	
	if (ac != 2) {
		std::cout << "usage: ./webserv <config_file.conf>" << std::endl;
		return (1);
	}
	try {
		config = configParser().parse(av[1]);
		//std::cout << "***** config print data at main *******" << std::endl;
		//config.printData();
		
		config.initServerRoutine(webserv, config.servers);
	}
	catch ( const std::exception &e ) {
		std::cerr << "Error (webserv): " << e.what() << std::endl;
		return (1);
	}
	std::cout << "webserv.run()" << std::endl;
	webserv.run(config);
	webserv.stop();
	
	return (0);
}