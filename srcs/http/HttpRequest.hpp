#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <map>
#include <string>
#include <iostream>

//REQUEST LINE: METHOD + TARGET(URL/URI){devide in two, path and query} + PROTOCOL_VERSION
class	HttpRequest {
	private:
		std::string	method_;
		std::string	url_;
		std::string	path_;
		std::string	query_;
		std::string	version_;
		std::string	body_;
		int			redirectCount_;
		std::map<std::string, std::string>	headers_;
		bool	isKeepAlive() const;
	public:
		HttpRequest();
		HttpRequest( const HttpRequest& other );
		HttpRequest &operator=(const HttpRequest& other);

		void	setMethod(const std::string &m);
		void	setUrl(const std::string &u);
		void	setPath(const std::string &p);
		void	setQuery(const std::string &q);
		void	setVersion(const std::string &v);
		void	setBody(const std::string &b);
		std::string	getMethod() const;
		std::string	getUrl() const;
		std::string	getPath() const;
		std::string	getQuery() const;
		std::string	getVersion() const;
		std::string	getBody() const;
		void	setHeader(const std::string &k, const std::string &v);
		bool	hasHeader(const std::string &key) const;
		std::string	getHeader(const std::string &key) const;
		const std::map<std::string, std::string> &getHeaders() const;
		void	incrementRedirectCount();
		int		getRedirectCount( ) const;
		void	setRedirectCount( int value );

		~HttpRequest();
};

#endif
