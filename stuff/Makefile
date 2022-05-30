NAME	=	webserv

CC		=	c++

RM		=	rm -f

CFLAGS	=	-Wall -Wextra -Werror -std=c++98

SRCS	=	srcs/webserv.cpp \
			srcs/config_handler.cpp \
			srcs/server.cpp \
			srcs/request.cpp \
			srcs/response.cpp \
			srcs/client.cpp \
			srcs/utils.cpp \
			srcs/cgi.cpp \
			srcs/parsing.cpp


INCLUDES	=	srcs/webserv.hpp \
				srcs/config_handler.hpp \
				srcs/server.hpp \
				srcs/request.hpp \
				srcs/response.hpp \
				srcs/client.hpp \
				srcs/cgi.hpp \
				srcs/parsing.hpp

OBJS	=	${SRCS:.cpp=.o}

${NAME}: ${OBJS} $(INCLUDES)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

all: ${NAME}

clean:
	$(RM) ${OBJS}

fclean: clean
	$(RM) ${NAME}

re: fclean all

test: all
	./${NAME}

.PHONY: all clean fclean re test
