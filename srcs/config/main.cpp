/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 00:37:26 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/02 21:35:08 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "globalConfig.hpp"
#include "configParser.hpp"
#include <iostream>

int	main(int ac, char **av)
{
	//take config file was argument
	globalConfig	config;
	
	(void)ac;
	try {
		config = configParser().parse(av[1]);
		config.printData();
	} 
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	//create functions to print the values
	// isolate them

	//search for edge cases to test
}