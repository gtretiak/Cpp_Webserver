/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 11:14:54 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/07 21:07:24 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef CGIREQUESTHANDLER_HPP
# define CGIREQUESTHANDLER_HPP

#include "../http/RequestHandler.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/globalConfig.hpp"
#include <string>
#include <cstring>
#include <utility>
#include <set>

class	CgiRequestHandler : public RequestHandler {
	private:
		std::map<std::string, std::string>	_meta_vars;
		static std::set<std::string>		_CgiMetaVarsList;
		
		const globalConfig*				_globalConfig;
		const serverConfig*				_serverSetting;
		const locationConfig*			_locSetting;
		char**							_envp;
	public:
		CgiRequestHandler();
		explicit CgiRequestHandler( const globalConfig* config );
		~CgiRequestHandler();

		void	handleRequest(HttpRequest &req, HttpResponse &res);//main call
		void	cgiExecutor( HttpResponse &res );

		char	**metaVarsToEnvp( );

		//globalConfig interface functions
		void					getConfigSettings( HttpRequest &req );
		const locationConfig*	findCgiLocation( const serverConfig& server, const std::string& pathTarget) const;
		//meta-variable getters
		void		extractMetaVars( HttpRequest& req );
		std::string	getPathInfo( std::string& requestUrl );
		std::string	getPathTranslated( );
		std::string	getMetaVar( std::string& key ) const ;
		//setters
		void							insertStaticMetaVars( );
		static std::set<std::string>&	initCgiMetaVars( );
		void							setMetaVar( std::string& key, std::string& value );
		//utilities
		void	printMetaVars( );
		void	printEnvp( );
};

void	printRequest(HttpRequest &req);
void	buildRequest(HttpRequest &req);

/*
REQUEST_METHOD      → "GET", "POST", "DELETE"  (from HTTP request line)
QUERY_STRING        → everything after ? in the URL (e.g. "name=foo&age=3")
CONTENT_TYPE        → from request header (e.g. "application/x-www-form-urlencoded")
CONTENT_LENGTH      → from request header, body size in bytes
PATH_INFO           → URI path after the script name (e.g. /cgi-bin/script.py/extra → "/extra")
PATH_TRANSLATED     → filesystem path of PATH_INFO (root + PATH_INFO)
SCRIPT_NAME         → URI path to the script itself ("/cgi-bin/script.py")
SCRIPT_FILENAME     → absolute filesystem path to the script ("/var/www/cgi-bin/script.py")
SERVER_NAME         → your server's hostname or IP
SERVER_PORT         → port your server is listening on ("8080")
SERVER_PROTOCOL     → "HTTP/1.1"
SERVER_SOFTWARE     → "webserv/1.0" (your server name)
GATEWAY_INTERFACE   → "CGI/1.1" (always this string)
REDIRECT_STATUS     → "200" (required by php-cgi specifically)
HTTP_*              → all HTTP request headers, prefixed with HTTP_
                      e.g. Cookie → HTTP_COOKIE
                          Accept → HTTP_ACCEPT
                          User-Agent → HTTP_USER_AGENT
*/


/*
The request-target must resolve to a file

That file must be mapped to a CGI interpreter

The file must be executable (or readable by the interpreter)
*/

//	create function to unchunk requests
//	create function to extract meta variables ok
//	then tries to execute:
//	the payload goes into stdin of the script
//	the script reads from stdin and execute
//	capture the script output from stdout
//	format the response into http response it has status code and content body

/* CGI EXECUTOR FLOW

1. Request arrives: POST /cgi-bin/upload.py?foo=bar
	we receive a HttpRequest& req;

2. Match location block → has cgi_extension .py /usr/bin/python3 
3. Build env vars from request headers + server config
4. If Transfer-Encoding: chunked → unchunk the body
5. pipe(stdin_pipe), pipe(stdout_pipe)
6. fork()
	 CHILD:
		 chdir(script_directory)
		 dup2(stdin_pipe[0],  STDIN_FILENO)
		 dup2(stdout_pipe[1], STDOUT_FILENO)
		 close unused pipe ends
		 execve("/usr/bin/python3", ["/usr/bin/python3", "/var/www/cgi-bin/upload.py", NULL], envp)
	 PARENT:
		 write body to stdin_pipe[1]
		 close stdin_pipe[1]          ← signals EOF to CGI
		 read stdout_pipe[0] until EOF ← collect CGI response
		 waitpid() to reap child
7. Parse CGI output (headers + body)
8. If no Content-Length in CGI output → use EOF-terminated body
9. Send HTTP response to client

*/

#endif
