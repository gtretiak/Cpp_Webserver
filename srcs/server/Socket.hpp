/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 20:15:46 by nogioni-          #+#    #+#             */
/*   Updated: 2026/08/31 00:15:59 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP
#include "../config/locationConfig.hpp"

struct	Listen;

class Socket
{
	private:
		int			_fd;
		bool		_isAbstract;
		std::string	_path;
	public:
		Socket();
		~Socket();

		void	create(int port); // receives the port where the server will listen from
		int		getFd() const;
		void	closeSocket();
		void	closeSocket(int fd);

		void	create(Listen target);

	private:
		void	setNonBlocking(int fd);
		void	setPortOnlyConn( Listen target);
		void	setIpHostPortConn( Listen target, Listen::listenType type );
		void	setUnixConn( Listen target );
};

#endif