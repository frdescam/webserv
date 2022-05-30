NAME	=	webserv

CC		=	c++

RM		=	rm -f

CFLAGS	=	-Wall -Wextra -Werror -std=c++98

SRCS	=	srcs/webserv.cpp \
			srcs/config/Config.cpp \
			srcs/server/Server.cpp \
			srcs/request/Request.cpp \
			srcs/response/Response.cpp \
			srcs/client/Client.cpp \
			srcs/utils/utils.cpp \
			srcs/cgi/Cgi.cpp \
			srcs/request/Parser.cpp


INCLUDES	=	inc/webserv.hpp \
				srcs/config/Config.hpp \
				srcs/server/Server.hpp \
				srcs/request/Request.hpp \
				srcs/response/Response.hpp \
				srcs/client/Client.hpp \
				srcs/cgi/Cgi.hpp \
				srcs/request/Parser.hpp

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
