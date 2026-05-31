NAME = run
FLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRC_DIR = srcs/
HTTP_DIR = http/
SRC = $(SRC_DIR)main.cpp \
      $(SRC_DIR)$(HTTP_DIR)ErrorPageGenerator.cpp $(SRC_DIR)$(HTTP_DIR)HttpParser.cpp $(SRC_DIR)$(HTTP_DIR)HttpRequest.cpp $(SRC_DIR)$(HTTP_DIR)HttpResponse.cpp $(SRC_DIR)$(HTTP_DIR)HttpException.cpp $(SRC_DIR)$(HTTP_DIR)StatusCodes.cpp $(SRC_DIR)$(HTTP_DIR)RequestHandler.cpp $(SRC_DIR)$(HTTP_DIR)StaticRequestHandler.cpp $(SRC_DIR)$(HTTP_DIR)CgiRequestHandler.cpp $(SRC_DIR)$(HTTP_DIR)MimeTypes.cpp $(SRC_DIR)$(HTTP_DIR)Router.cpp

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
