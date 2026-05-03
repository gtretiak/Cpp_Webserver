#ifndef STATUSCODES_HPP
#define STATUSCODES_HPP
#include <string>

class	StatusCodes {
	public:
		StatusCodes();
		static const std::string	&getStatus(int code);
		~StatusCodes();
};

#endif
