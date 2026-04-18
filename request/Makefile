NAME = request

CFLAGS = -Wextra -Wall -Werror -std=c++98 -fsanitize=address -g3

CC = c++

SRC = main.cpp request.cpp

SRC_H = request.hpp

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