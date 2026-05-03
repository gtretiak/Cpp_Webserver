/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:32 by gtretiak          #+#    #+#             */
/*   Updated: 2026/05/03 19:03:19 by gtretiak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>

struct	Connection;
class	HttpRequest;

class	HttpParser {
	private:
//		HttpHandler	handler;
		void	parseLine(std::string &buf, HttpRequest *req);
		void	parseHeaders(std::string &buf, HttpRequest *req);
		void	parseBody(std::string &buf, HttpRequest *req);
	public:
		bool	isRequestComplete(const Connection &conn) const;
		size_t	parseRequest(std::string &buf, HttpRequest *req);
};

#endif
