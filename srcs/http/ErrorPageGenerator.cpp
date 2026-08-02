#include "ErrorPageGenerator.hpp"
#include "HttpUtils.hpp"
#include "StatusCodes.hpp"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

static std::string	tryRead(const std::string &filepath) {
	int	fd = open(filepath.c_str(), O_RDONLY);
	if (fd < 0)
		return "";
	char	buf[BUFFER_SIZE];
	std::string	content;
	ssize_t	bytesRead = 1;
	while (bytesRead > 0)
	{
		bytesRead = read(fd, buf, sizeof(buf));
		if (bytesRead <= 0)
			break ;
		content.append(buf, bytesRead);
	}
	close(fd);
	if (bytesRead < 0)
		return "";
	return content;
}

std::string	ErrorPageGenerator::generate(int code) {
	std::ostringstream	path;
	path << "../../www/errors/" << code << ".html";
	std::string	page = tryRead(path.str());
	if (!page.empty())
		return page;
	std::ostringstream	secondOption;
	secondOption << "<html><head><title>"
	             << code << " " << StatusCodes::getStatus(code)
        	     << "</title></head><body><h1>"
	             << code << " " << StatusCodes::getStatus(code)
        	     << "</h1></body></html>";
	return secondOption.str();
}
