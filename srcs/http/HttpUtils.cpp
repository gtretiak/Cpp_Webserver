#include "HttpUtils.hpp"
#include <string>

std::string	toLower(const std::string &key) {
	std::string	res = key;
	for (size_t i = 0; i < res.size(); i++)
		res[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(res[i])));
	return (res);
}
