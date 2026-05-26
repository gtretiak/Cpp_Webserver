/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:32 by gtretiak          #+#    #+#             */
/*   Updated: 2026/05/26 12:54:10 by gtretiak         ###   ########.fr       */
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
		void	parseLine(const std::string &buf, HttpRequest *req);
		void	parseHeaders(std::string &buf, HttpRequest *req);
		void	parseBody(std::string &buf, HttpRequest *req);
	public:
		HttpParser();
		bool	isRequestComplete(const std::string &readBuffer) const;
		size_t	parseRequest(std::string &buf, HttpRequest *req);
		~HttpParser();
};

#endif
