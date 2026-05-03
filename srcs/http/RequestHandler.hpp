#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

class	RequestHandler {
	public:
		RequestHandler();
		void	handleRequest(const Connection &conn) const;
		~RequestHandler();
};

#endif
