/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_execution.cpp                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 17:42:33 by dopereir          #+#    #+#             */
/*   Updated: 2026/07/16 00:05:00 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"
#include "../server/Connection.hpp"

/// @brief To be use only in execution context, it gets the root repository
/// @brief derived from location block or server block setting
/// @return throws a exception if both settings has invalid root directives.
std::string	CgiRequestHandler::getExecRoot( ) {
	std::string	root;

	if (!_locSetting->_root.empty()) {
		root = _locSetting->_root;
		return root;
	}
	else if (!_serverSetting->_root.empty()) {
		root = _serverSetting->_root;
		return root;
	}
	else
		throw HttpException(500, "no root directive defined");
}

/// @brief Combines the root + URL path to create a valid path to script
/// @param root root path to execution context
/// @param url request target from http request line
/// @return returns a valid script path derived from root and request line
std::string CgiRequestHandler::getExecScriptPath( std::string& root, std::string url ) {
	std::string	scriptPath;

	if (!root.empty() && root[root.size() - 1] == '/' && !url.empty() && url[0] == '/')
		scriptPath = root + url.substr(1);
	else if (!root.empty() && root[root.size() - 1] != '/' && !url.empty() && url[0] != '/')
		scriptPath = root + "/" + url;
	else
		scriptPath = root + url;
	return (scriptPath);
}

std::string CgiRequestHandler::getExecScriptDir( std::string& scriptPath ) {
	std::string	scriptDir;

	scriptDir = scriptPath.substr(0, scriptPath.find_last_of('/'));
	return scriptDir;
}

std::string CgiRequestHandler::getExecScriptName( std::string& scriptPath ) {
	std::string	scriptName;

	scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);
	return scriptName;
}

/*void	writeRequestBodyToCgi( HttpRequest& req, int stdin_pipe[2] ) {
	const std::string	requestBody = req.getBody();
	size_t				written;

	if (!requestBody.empty()) {
		const char	*buffer = requestBody.c_str();
		written = 0;
		while (written < requestBody.size()) {
			ssize_t chunk = write(stdin_pipe[1], buffer + written, requestBody.size() - written);
			if (chunk <= 0)
				break;
			written += static_cast<size_t>(chunk);
		}
	}
}*/

std::string	CgiRequestHandler::readCgiOutput( int stdout_pipe ) {
	std::string	cgiOutput;
	char	buf[4096];
	ssize_t	bytes;

	while ((bytes = read(stdout_pipe, buf, sizeof(buf))) > 0) {
		cgiOutput.append(buf, static_cast<size_t>(bytes));
	}
	return cgiOutput;
}

void	CgiRequestHandler::setExecContext( t_ctx_exec& ctx, HttpRequest& req ) {
	ctx.execRoot = getExecRoot( );
	ctx.scriptPath = getExecScriptPath( ctx.execRoot, req.getPath());

	//std::cout << "DEBUG: " << ctx.scriptPath << std::endl;
	//std::cout << "DEBUG: " << req.getPath() << std::endl;
	
	if (access(ctx.scriptPath.c_str(), F_OK) != 0)
		throw HttpException(404, "CGI script not found");
	if (access(ctx.scriptPath.c_str(), X_OK) != 0)
		throw HttpException(403, "CGI script not executable");
	ctx.scriptDir = getExecScriptDir(ctx.scriptPath);
	ctx.scriptName = getExecScriptName(ctx.scriptPath);

	//std::cout << "DEBUG: " << ctx.scriptDir << std::endl;
	//std::cout << "DEBUG: " << ctx.scriptName << std::endl;

	if (pipe(ctx.stdin_pipe) == -1)
		throw HttpException(500, "pipe creation failed");
	if (pipe(ctx.stdout_pipe) == -1) {
		close(ctx.stdin_pipe[0]);
		close(ctx.stdin_pipe[1]);
		throw HttpException(500, "pipe creation failed");
	}
}

void	CgiRequestHandler::childRun( t_ctx_exec& ctx ) {
	char	*argv[2];

	if (chdir(ctx.scriptDir.c_str()) != 0)
		_exit(1);
	if (dup2(ctx.stdin_pipe[0], STDIN_FILENO) == -1) {
		_exit(1);
	}
	if (dup2(ctx.stdout_pipe[1], STDOUT_FILENO) == -1) {
		_exit(1);
	}
	close(ctx.stdin_pipe[1]);
	close(ctx.stdout_pipe[0]);
	close(ctx.stdin_pipe[0]);
	close(ctx.stdout_pipe[1]);
	argv[0] = const_cast<char*>(ctx.scriptName.c_str());
	argv[1] = NULL;
	execve(ctx.scriptName.c_str(), argv, _envp);
	_exit(1);
}


void	CgiRequestHandler::cgiExecutor( Connection& conn ) {
	t_ctx_exec	ctx;
	pid_t		pid;

	const std::string	requestBody = conn.req.getBody();

	if (!_envp) {
		throw HttpException(500, "Internal Error envp not valid");
	}
	setExecContext(ctx, conn.req);
	pid = fork();
	if (pid == -1) {
		close(ctx.stdin_pipe[0]);
		close(ctx.stdin_pipe[1]);
		close(ctx.stdout_pipe[0]);
		close(ctx.stdout_pipe[1]);
		throw HttpException(500, "fork failed");
	}
	if (pid == 0) {
		childRun(ctx);
	}

	close(ctx.stdin_pipe[0]);
	close(ctx.stdout_pipe[1]);

	conn.cgiData.pid = pid;
	conn.cgiData.inFd = ctx.stdin_pipe[1];
	conn.cgiData.outFd = ctx.stdout_pipe[0];
	conn.cgiData.cgiLastActivity = time(NULL) + 10;

	conn.cgiData.pollFd.fd = conn.cgiData.outFd;
	conn.cgiData.pollFd.events = POLLIN;
	conn.cgiData.pollFd.revents = 0;
}
