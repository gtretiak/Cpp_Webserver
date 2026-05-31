NAME = run
FLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRC_DIR = srcs/
HTTP_DIR = http/
CGI_DIR = cgi/
CONFIG_DIR = config/
SERVER_DIR = server/
SRC = $(SRC_DIR)main.cpp \
      $(SRC_DIR)$(HTTP_DIR)ErrorPageGenerator.cpp $(SRC_DIR)$(HTTP_DIR)HttpParser.cpp $(SRC_DIR)$(HTTP_DIR)HttpRequest.cpp $(SRC_DIR)$(HTTP_DIR)HttpResponse.cpp $(SRC_DIR)$(HTTP_DIR)HttpException.cpp $(SRC_DIR)$(HTTP_DIR)StatusCodes.cpp $(SRC_DIR)$(HTTP_DIR)RequestHandler.cpp $(SRC_DIR)$(HTTP_DIR)StaticRequestHandler.cpp $(SRC_DIR)$(HTTP_DIR)MimeTypes.cpp $(SRC_DIR)$(HTTP_DIR)Router.cpp \
      $(SRC_DIR)$(CGI_DIR)CgiRequestHandler.cpp \
#     $(SRC_DIR)$(SERVER_DIR) \ put your server source .cpp files here TODO
#     $(SRS_DIR)$(CONFIG_DIR) put your config source .cpp files here TODO

OBJ = $(addprefix ./, $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	@c++ $(OBJ) -o $(NAME)
	@echo "Compiled successfully!"

%.o:%.cpp
	@c++ $(FLAGS) -c $< -o $@

.PHONY: clean fclean re

clean:
	@rm -rf $(OBJ)
	@echo "Objects removed"

fclean: clean
	@rm -rf $(NAME)
	@echo "Program removed"

re: fclean all
