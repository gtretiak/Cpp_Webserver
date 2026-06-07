#include "CgiRequestHandler.hpp"
#include "../http/HttpException.hpp"
#include "../config/configParser.hpp"
#include <iostream>

int main(int ac, char **av) {
	globalConfig	config;
	HttpRequest req;

	if (ac != 2) {
		std::cout << "./cgi_demo <config_file>" << std::endl;
		return (1);
	}
	try {
		config = configParser().parse(av[1]);

		buildRequest(req);
		printRequest(req);

		CgiRequestHandler	cgi(&config);

		cgi.extractMetaVars(req);
		std::cout << "\n*************** printMetaVars() *************** " << std::endl;
		cgi.printMetaVars();
		std::cout << "\n*************** printEnvp() *************** " << std::endl;
		cgi.printEnvp();
	}
	catch (const HttpException &e) {
		std::cerr << "Failed to build CGI request: " << e.code()
			<< " " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
