/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequestHandler_execution.cpp                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dopereir <dopereir@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/10 17:42:33 by dopereir          #+#    #+#             */
/*   Updated: 2026/08/27 22:47:51 by dopereir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequestHandler.hpp"
#include "../http/MimeTypes.hpp"
#include "../http/HttpException.hpp"
#include "../server/Connection.hpp"
#include <limits.h>

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
}//DEPRECATED

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

std::string	CgiRequestHandler::buildCgiFilePath( const std::string &root, const std::string &path ) const {
	std::string	remainder;
	std::string	locPrefix = _locSetting->_path;

	if (path.compare(0, locPrefix.size(), locPrefix) == 0)
		remainder = path.substr(locPrefix.size());
	else
		remainder = path;

	if (root.empty())
		throw HttpException(500, "Empty root directive");

	if (!root.empty() && root[root.size() - 1] == '/' && !remainder.empty() && remainder[0] == '/')
		return root + remainder.substr(1);

	if (!root.empty() && root[root.size() - 1] != '/' && !remainder.empty() && remainder[0] != '/')
		return root + "/" + remainder;

	return root + remainder;
}

void	CgiRequestHandler::setExecContext( t_ctx_exec& ctx, HttpRequest& req ) {
	ctx.execRoot = getExecRoot( );
	ctx.scriptPath = buildCgiFilePath( ctx.execRoot, req.getPath());

	//std::cout << "DEBUG: buildCgiFilePath:\t" << ctx.scriptPath << std::endl;
	//std::cout << "DEBUG: req.getPath():\t" << req.getPath() << std::endl;
	
	//instead of check for the name of resource file, check the existence of interpreter/cgi_tester
	if (access(ctx.cgiExecutable.c_str(), X_OK) != 0)
		throw HttpException(500, "CGI script not executable");
	ctx.scriptDir = getExecScriptDir(ctx.scriptPath);
	ctx.scriptName = getExecScriptName(ctx.scriptPath);

	//std::cout << "DEBUG: getExecScriptDir(scriptDir):\t" << ctx.scriptDir << std::endl;
	//std::cout << "DEBUG: getExecScriptName(scriptName):\t" << ctx.scriptName << std::endl;

	if (pipe(ctx.stdin_pipe) == -1)
		throw HttpException(500, "pipe creation failed");
	if (pipe(ctx.stdout_pipe) == -1) {
		close(ctx.stdin_pipe[0]);
		close(ctx.stdin_pipe[1]);
		throw HttpException(500, "pipe creation failed");
	}
}

void	CgiRequestHandler::childRun( t_ctx_exec& ctx ) {
	char	*argv[3];

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
	argv[0] = const_cast<char*>(ctx.cgiExecutable.c_str());
	argv[1] = const_cast<char*>(ctx.scriptName.c_str());
	argv[2] = NULL;

	//maybe the solving is to cgiExecutable be a absolute path to the script
	execve(ctx.cgiExecutable.c_str(), argv, _envp);
	_exit(1);
}

static std::string	resolveExecutable( const std::string& configured )
{
	char absolutePath[PATH_MAX];

	if (realpath(configured.c_str(), absolutePath) == NULL)
		throw HttpException(500, "CGI executable not found");
	if (access(absolutePath, X_OK) != 0)
		throw HttpException(500, "CGI executable is not executable");
	return std::string(absolutePath);
}

void	CgiRequestHandler::cgiExecutor( Connection& conn ) {
	t_ctx_exec	ctx;
	pid_t		pid;

	if (!_envp) {
		throw HttpException(500, "Internal Error envp not valid");
	}

	ctx.cgiExecutable = resolveExecutable(conn.cgiExecutable);
	//std::cout << "\t\tDEBUG: cgiExecutor(): ctx.cgiExecutable: " << ctx.cgiExecutable << std::endl;
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

	fcntl(ctx.stdin_pipe[1], F_SETFL, O_NONBLOCK);
	fcntl(ctx.stdout_pipe[0], F_SETFL, O_NONBLOCK);

	conn.cgiData.pid = pid;
	conn.cgiData.inFd = ctx.stdin_pipe[1];
	conn.cgiData.outFd = ctx.stdout_pipe[0];
	conn.cgiData.cgiLastActivity = time(NULL) + 10;

	conn.cgiData.pollFd.fd = conn.cgiData.outFd;
	conn.cgiData.pollFd.events = POLLIN;
	conn.cgiData.pollFd.revents = 0;
}
