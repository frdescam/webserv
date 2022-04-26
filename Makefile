NAME = webserv

SRCS += main.cpp

OBJS = $(SRCS:.cpp=.o)

CC = c++

RM = rm -f

#MULTITHREADED BUILD FLAGS#
#MAKEFLAGS+=-j8

CPP_FLAGS += -Wall -Wextra -Werror

CPP_FLAGS += -g
CPP_FLAGS += -fsanitize=address

CGI_OUT = http://127.0.0.1::8080/scripts/layout.html

#nav:
#	firefox $(CGI_OUT)

#goo-nav:
#	google-chrome $(CGI_OUT)

%.o:		%.cpp
			$(CC) $(CPP_FLAGS) -c $< -o $@

$(NAME):	$(OBJS)
			$(CC) $(CPP_FLAGS) -o $(NAME) $(OBJS)

all:		$(NAME)

clean:
			$(RM) $(OBJS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

test:		all
			./$(NAME)

.PHONY: all clean fclean re test
