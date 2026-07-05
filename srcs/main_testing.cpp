/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_testing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 21:38:29 by dopereir          #+#    #+#             */
/*   Updated: 2026/07/02 23:25:16 by dopereir         ###   ########.fr       */
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

	if (ac != 2) {
		std::cout << "usage: ./webserv <config_file.conf>" << std::endl;
		return (1);
	}
	try {//JUST SETUP THINGS BASED ON CONFIG FILE, NO RUNNING YET
		config = configParser().parse(av[1]);
		

		config.initServerRoutine(webserv, config.servers);
	}
	catch ( const std::exception &e ) {
		std::cerr << "Error (webserv): " << e.what() << std::endl;
		return (1);
	}
	std::cout << "webserv.run()" << std::endl;
	webserv.run(config);
	
	return (0);
}