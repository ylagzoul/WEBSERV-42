NAME = server

CFLAGS = -Wextra -Wall -Werror -std=c++98 #-fsanitize=address -g
#-fsanitize=address
CC = c++

SRC = server.cpp \
			../request/request.cpp \
			../response/Response.cpp \
			../config_file/config_file.cpp \
			../config_file/Helpful_functions/get_set_conf.cpp \
			../config_file/Helpful_functions/ft_ft.cpp \
			../config_file/line/listen.cpp \
			../config_file/line/server_name.cpp \
			../config_file/line/error_page.cpp

SRC_H = server.hpp 

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