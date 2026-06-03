/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 00:37:26 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/03 13:08:39 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "globalConfig.hpp"
#include "configParser.hpp"
#include <iostream>

int	main(int ac, char **av)
{
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
}