NAME = webserv
NAMED = webserv
SRC = webserv.cpp ConfigParser.cpp
CC = c++
CCD = clang++
CFLAGS = -Wall -Wextra -Werror -pedantic -Wshadow -g -std=c++98 -fsanitize=address,undefined
CFLAGSD = -Wall -Wextra -Werror -pedantic -Wshadow -g -fno-limit-debug-info -std=c++98 -fsanitize=address,undefined

all:	$(NAME)

$(NAME): $(SRC)
		 $(CC) $(CFLAGS) $(^) -o $(NAME)

debug:  $(NAMED)

$(NAMED): $(SRC)
		  $(CCD) $(CFLAGSD) $(^) -o $(NAMED)

clean:
			@rm -f $(OBJS)

fclean:		clean
			@rm -f $(NAME)

re:			fclean all

.PHONY:		all clean re fclean