NAME = webserv

COMP = c++

CFLAGS = #-Wall -Wextra -Werror -std=c++17

SRC = main.cpp ConnectionHandler.cpp ResponseHandler.cpp

OBJ = $(SRC:.cpp=.o)

.PHONY: all re clean fclean

all: $(NAME)

$(NAME): $(OBJ)
	$(COMP) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(COMP) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
