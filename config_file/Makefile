NAME    = config_file
CC      = c++
FLAG    = -Wextra -Wall -Werror -std=c++98
SRC     = config_file.cpp Helpful_functions/get_set_conf.cpp Helpful_functions/ft_ft.cpp line/listen.cpp line/server_name.cpp line/error_page.cpp
OBJ     = $(SRC:%.cpp=%.o)
HEADERS = config_file.hpp

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAG) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAG) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all