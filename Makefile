NAME = run
FLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRC_DIR = srcs/
OBJ_DIR = obj/

HTTP_DIR = http/
CGI_DIR = cgi/
CONFIG_DIR = config/
SERVER_DIR = server/

CONFIG_SRCS =	configParser.cpp \
				locationConfig.cpp \
				serverConfig.cpp

HTTP_SRCS =		HttpParser.cpp \
				HttpRequest.cpp \
				HttpResponse.cpp \
				HttpException.cpp \
				StatusCodes.cpp \
				RequestHandler.cpp \
				StaticRequestHandler.cpp \
				MimeTypes.cpp \
				Router.cpp \
				ErrorPageGenerator.cpp

CGI_SRCS =	CgiRequestHandler.cpp

#SERVER_SRCS = PUT THE SERVER FOLDER SRCS FILES HERE


SRC =	$(SRC_DIR)main.cpp \
		$(addprefix $(SRC_DIR)$(HTTP_DIR), $(HTTP_SRCS)) \
		$(addprefix $(SRC_DIR)$(CGI_DIR), $(CGI_SRCS)) \
		$(addprefix $(SRC_DIR)$(CONFIG_DIR), $(CONFIG_SRCS)) \
		#$(addprefix $(SRC_DIR)$(CGI_DIR), $(CGI_SRCS))

OBJ = $(addprefix $(OBJ_DIR), $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	@c++ $(OBJ) -o $(NAME)
	@echo "Compiled successfully!"

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	@c++ $(FLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)
	@echo "Objects removed"

fclean: clean
	@rm -rf $(NAME)
	@echo "Program removed"

re: fclean all

.PHONY: clean fclean re