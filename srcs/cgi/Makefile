CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I../http -I.

TARGET = cgi_demo
SRCS = main.cpp \
	CgiRequestHandler.cpp \
	CgiRequestHandler_config.cpp \
	CgiRequestHandler_execution.cpp \
	CgiRequestHandler_redirect.cpp \
	CgiRequestHandler_utils.cpp \
	../http/HttpRequest.cpp \
	../http/HttpResponse.cpp \
	../http/HttpException.cpp \
	../http/RequestHandler.cpp \
	../http/StatusCodes.cpp \
	../config/globalConfig.cpp \
	../config/locationConfig.cpp \
	../config/serverConfig.cpp \
	../config/configParser.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
