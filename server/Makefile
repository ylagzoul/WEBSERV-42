NAME = server

CFLAGS = -Wextra -Wall -Werror -std=c++98 -g3

CC = c++

SRC = ../request/main.cpp ../request/request.cpp server.cpp

SRC_H = ../request/request.hpp server.hpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(SRC_H)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all