#include "CgiRequestHandler.hpp"
#include "../http/Router.hpp"
#include "../http/HttpException.hpp"
#include "../config/configParser.hpp"
#include <iostream>

int main(int ac, char **av) {
	globalConfig	config;
	HttpRequest		req;
	HttpResponse	res;

	if (ac != 2) {
		std::cout << "./cgi_demo <config_file>" << std::endl;
		return (1);
	}
	try {
		config = configParser().parse(av[1]);

		Router	router(&config);

		buildRequest(req);
		std::cout << "\n*************** printRequest() *************** " << std::endl;
		printRequest(req);

		//CgiRequestHandler	cgi(&config);

		//cgi.handleRequest(req, res);
		/*std::cout << "\n*************** printMetaVars() *************** " << std::endl;
		cgi.printMetaVars();
		std::cout << "\n*************** printResponse() *************** " << std::endl;
		printResponse(res);*/

		std::cout <<"\n************** resolve() **************" << std::endl;
		router.resolve(req, res);
		std::cout << "\n*************** printResponse() *************** " << std::endl;
		printResponse(res);
	}
	catch (const HttpException &e) {
		std::cerr << "Failed to build CGI request: " << e.code()
			<< " " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

