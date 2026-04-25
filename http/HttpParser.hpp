#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

class	HttpParser {
	public:
		bool	isRequestComplete(const Connection &conn) const;
};

#endif
