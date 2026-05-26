NAME = run
FLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRC = main.cpp HttpParser.cpp HttpRequest.cpp HttpResponse.cpp HttpException.cpp StatusCodes.cpp RequestHandler.cpp StaticRequestHandler.cpp
OBJ = $(addprefix ./, $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	c++ $(OBJ) -o $(NAME)

%.o:%.cpp
	c++ $(FLAGS) -c $< -o $@

.PHONY: clean fclean re

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf $(NAME)

re: fclean all
