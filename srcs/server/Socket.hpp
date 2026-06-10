/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nogioni- <nogioni-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 20:15:46 by nogioni-          #+#    #+#             */
/*   Updated: 2026/06/10 20:36:18 by nogioni-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

class Socket
{
	private:
		int	_fd; 
	public:
		Socket();
		~Socket();

		void create(int port); // receives the port where the server will listen from
		int getFd() const;
		void closeSocket();

		private:
			void	setNonBlocking(int fd);
};

#endif