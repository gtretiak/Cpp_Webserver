/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WIP_HttpParser.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:32 by gtretiak          #+#    #+#             */
/*   Updated: 2026/04/27 16:32:10 by gtretiak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

class	HttpParser {
	private:
		HttpHandler	handler;
	public:
		bool	isRequestComplete(const Connection &conn) const;
		void	parseRequest(std::string &buf, HttpRequest *req);
		void	parseLine(std::string &buf, HttpRequest *req);
		void	parseHeaders(std::string &buf, HttpRequest *req);
		void	parseBody(std::string &buf, HttpRequest *req);
};

#endif
