NAME = server
CC = c++
FLAGE = -Wextra -Wall -Werror -std=c++98
SRC = main.cpp server.cpp
OBJ = $(SRC:%.cpp=%.o)
HEADERS = server.hpp

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(GLAG) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAG) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all