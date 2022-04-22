SRC += main.cpp
SRC += cgi_handler.cpp
SRC += config_checker.cpp
SRC += server.cpp
SRC += request_handler.cpp
SRC += client_handler.cpp

OBJ = $(SRC:.cpp=.o)

CC = c++

RM = rm -f

NAME = webserv

#MULTITHREADED BUILD FLAGS#
#MAKEFLAGS+=-j8

CGI_OUT = http://127.0.0.1::8080/scripts/layout.html

ifeq ($(df), 0)
CPP_FLAGS += -Wall
CPP_FLAGS += -Werror
CPP_FLAGS += -Wextra
CPP_FLAGS += -std=c++98
endif

CPP_FLAGS += -g
CPP_FLAGS += -fsanitize=address

all:	build

exec:	run nav

build:	$(NAME)

run:
	./$(NAME)

nav:
	firefox $(CGI_OUT)

goo-nav:
	google-chrome $(CGI_OUT)

$(NAME):	$(OBJ)
	$(CC) $(CPP_FLAGS) -o $(NAME) $(OBJ)

%.o:	%.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re: fclean all

.PHONY: clean fclean re
