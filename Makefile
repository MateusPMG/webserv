NAME = webserv
SRC = webserv.cpp ConfigParser.cpp
CC = @c++
CFLAGS = -Wall -Wextra -Werror -g -std=c++98 #-fsanitize=address,undefined

all:	$(NAME)

$(NAME): $(SRC)
		 $(CC) $(CFLAGS) $(^) -o $(NAME)

clean:
			@rm -f $(OBJS)

fclean:		clean
			@rm -f $(NAME)

re:			fclean all

.PHONY:		all clean re fclean