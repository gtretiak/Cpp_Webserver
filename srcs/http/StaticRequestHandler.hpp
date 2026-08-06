#ifndef STATICREQUESTHANDLER_HPP
#define STATICREQUESTHANDLER_HPP

#include "RequestHandler.hpp"
#include "../config/globalConfig.hpp"
#include "../config/serverConfig.hpp"
#include "../config/locationConfig.hpp"

class StaticRequestHandler : public RequestHandler
{
private:
	globalConfig *_config;
	serverConfig *_server;
	locationConfig *_location;

	static std::string readFile(const std::string &path);
	static std::string getUniquePath(const std::string &path);
	static std::string getFileName(const std::string &path);
	static void writeFile(const std::string &path, const std::string &content);
	static void deleteFile(const std::string &path);
	static bool fileExists(const std::string &path);
	static bool hasWDPermission(const std::string &path);

	std::string getRoot() const;
	std::string buildFilePath(const std::string &root, const std::string &path) const;
	bool isDirectory(const std::string &path) const;
	bool isRegularFile(const std::string &path) const;
	std::string resolveIndexFile(const std::string &dirPath) const;

	bool getAutoindex() const;
	std::string generateAutoindexPage(const std::string &requestPath, const std::string &dirPath) const;

public:
	StaticRequestHandler();

	void setConfig(globalConfig *config);
	void setContext(serverConfig *server, locationConfig *location);
	void handleRequest(HttpRequest &req, HttpResponse &res);

	~StaticRequestHandler();
};

#endif