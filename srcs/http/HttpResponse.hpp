#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <string>
#include <map>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

class	HttpResponse {
	private:
		std::string	version_;
		std::string	serverName_;
		int			statusCode_;
		std::string	statusText_;
		std::string	body_;
		std::string	date_;
		std::map<std::string, std::string>	headers_;
		bool		has_status_;
	public:
		HttpResponse();
		HttpResponse(const std::string &serverName);
		HttpResponse	&operator=(const HttpResponse &other);

		std::string	toString() const;
		void		setVersion(const std::string &v);
		void		setStatus(int code);
		void		setStatusText( );
		void		setStatusBool( bool setting );
		void		setBody(const std::string &b);
		void		setHeader(const std::string &k, const std::string &v);

		bool		hasStatusCode( );

		std::string			getHeader(const std::string &k) const;
		const std::map<std::string, std::string>&	getHeaders( ) const;
		int					getStatusCode();
		const std::string	getVersion( ) const;
		const std::string	getBody( ) const;
		

		bool		hasHeader(const std::string &k) const;
		void		generateErrorPageResponse( const char *filepath, int errorCode );
		~HttpResponse();
};

#endif
