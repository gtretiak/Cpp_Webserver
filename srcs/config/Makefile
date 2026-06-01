CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I.

SRCS = configParser.cpp globalConfig.cpp locationConfig.cpp serverConfig.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = config_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
