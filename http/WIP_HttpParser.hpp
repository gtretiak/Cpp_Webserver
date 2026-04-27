/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WIP_HttpParser.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gtretiak <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:42:32 by gtretiak          #+#    #+#             */
/*   Updated: 2026/04/27 11:10:22 by gtretiak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

class	HttpParser {
	private:
		HttpRequest	request;
		HttpHandler	handler;
	public:
		bool	isRequestComplete(const Connection &conn) const;
		HttpRequest	&parse(std::string &buf);
};

#endif
