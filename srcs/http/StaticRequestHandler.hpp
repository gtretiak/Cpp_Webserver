#ifndef STATICREQUESTHANDLER_HPP
#define STATICREQUESTHANDLER_HPP

#include "RequestHandler.hpp"

class	StaticRequestHandler : public RequestHandler {
	private:
		static std::string	readFile(const std::string &path);
		static std::string	getUniquePath(const std::string &path);
		static std::string	getFileName(const std::string &path);
		static void	writeFile(const std::string &path, const std::string &content);
		static void	deleteFile(const std::string &path);
		static bool	fileExists(const std::string &path);
		static bool	hasWDPermission(const std::string &path);
	public:
		StaticRequestHandler();
		void	handleRequest(HttpRequest &req, HttpResponse &res);
		//void	handleRequest(HttpRequest &req, HttpResponse &res, LocationConfig *loc); TODO
		~StaticRequestHandler();
};

#endif
