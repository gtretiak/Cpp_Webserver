#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP
#include <string>
#include <cctype>

const size_t BUFFER_SIZE = 4096;

std::string	toLower(const std::string &key);
std::string	ft_int_to_string(int n);
std::string	createPrettyErrorPage(int code);

#endif
