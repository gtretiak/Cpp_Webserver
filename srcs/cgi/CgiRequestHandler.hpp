/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 11:14:54 by dopereir          #+#    #+#             */
/*   Updated: 2026/06/28 19:29:47 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef CGIREQUESTHANDLER_HPP
# define CGIREQUESTHANDLER_HPP

#include "../http/RequestHandler.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/globalConfig.hpp"
#include <iostream>
#include <cctype>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <utility>
#include <set>

class	Router;
class	Connection;

typedef struct	s_ctx_exec {
	std::string	execRoot;
	std::string	scriptPath;
	std::string	scriptDir;
	std::string	scriptName;
	int			stdin_pipe[2];
	int			stdout_pipe[2];
	std::string	cgiOutput;
}	t_ctx_exec;

class	CgiRequestHandler : public RequestHandler {
	private:
		typedef enum e_CgiResponseType {
			CGI_DOCUMENT,
			CGI_LOCAL_REDIR,
			CGI_CLIENT_REDIR,
			CGI_CLIENT_DOC_REDIR,
			NONE
		}	cgiResponseType;

	
		std::map<std::string, std::string>	_meta_vars;
		static std::set<std::string>		_CgiMetaVarsList;
		
		globalConfig*				_globalConfig;
		serverConfig*				_serverSetting;
		locationConfig*				_locSetting;
		char**						_envp;

	public:
		CgiRequestHandler();
		explicit CgiRequestHandler( globalConfig* config );
		~CgiRequestHandler();

		//handleRequest main call
		void	handleRequest(Connection& conn);
		void	handleRequest( HttpRequest &req, HttpResponse &res );

		//transitions
		void	finalizeCgi(Connection& conn);

		//execution
		void		setExecContext( t_ctx_exec& ctx, HttpRequest& req );
		void		childRun( t_ctx_exec& ctx );
		void		cgiExecutor( Connection& conn );
		std::string	readCgiOutput( int stdout_pipe );
		std::string	getExecRoot( );
		std::string	getExecScriptPath( std::string& root, std::string url );
		std::string	getExecScriptDir( std::string& scriptPath );
		std::string	getExecScriptName( std::string& scriptPath );
		
		//merge configs
		void		extractHeader( std::string line, size_t colon, HttpResponse &res );
		void		parseHeaderSection( std::string& headerSection, HttpResponse& res );
		void		parseCgiHttpResponse( HttpResponse &res, std::string &cgiOutput );

		//redirects handlers
		cgiResponseType	classifyCgiResponse( HttpResponse& res );
		HttpRequest		processLocalRedir( HttpResponse& res );
		void			processClientRedir( HttpResponse& res );
		void			processClientRedirWithDocument( HttpResponse& res );
		

		//meta-variables operations
		void		extractMetaVars( HttpRequest& req );
		char		**metaVarsToEnvp( );
		void		freeEnvp( );

		//globalConfig interface functions
		void					getConfigSettings( HttpRequest &req );
		locationConfig*			findCgiLocation( serverConfig& server, const std::string& pathTarget) const;
		int						getClientMaxBodySize( );

		//meta-variable getters
		std::string	getPathInfo( std::string& requestUrl );
		std::string	getScriptName( std::string& requestUrl );
		std::string	getPathTranslated( );
		std::string	getMetaVar( std::string& key ) const ;
		std::string	getQueryFromURI( const std::string& URI );
		std::string	getPathFromURI( const std::string& URI );
		
		//setters
		static std::set<std::string>&	initCgiMetaVars( );
		void							insertStaticMetaVars( );
		void							setMetaVar( std::string& key, std::string& value );
		void							setConfig( globalConfig* config );
		
		//utilities
		void	printMetaVars( );
		void	printEnvp( );
};

//interface functions for eventloop
//void	writeRequestBodyToCgi( HttpRequest &req, int stdin_pipe[2] );

void	printRequest( HttpRequest &req );
void	printResponse( HttpResponse &res );
void	buildRequest( HttpRequest &req );


/*
The request-target must resolve to a file

That file must be mapped to a CGI interpreter

The file must be executable (or readable by the interpreter)
*/

//	create function to unchunk requests

#endif
