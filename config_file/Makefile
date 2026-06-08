NAME = configll

CFLAGS = -Wextra -Wall -Werror -std=c++98 -g  #-fsanitize=address -g # -fsanitize=address,undefined -g -O1 
CC = c++

SRC = main.cpp config_file.cpp \
			Helpful_functions/get_set_conf.cpp \
			Helpful_functions/ft_ft.cpp \
			line/listen.cpp \
			line/server_name.cpp \
			line/error_page.cpp

SRC_H =  config_file.hpp


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