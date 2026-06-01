/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   globalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 00:35:52 by dopereir          #+#    #+#             */
/*   Updated: 2026/05/28 23:45:42 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GLOBALCONFIG_HPP
# define GLOBALCONFIG_HPP
# include "serverConfig.hpp"

struct	globalConfig {
	std::vector<serverConfig>	servers;
};

#endif