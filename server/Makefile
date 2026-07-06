NAME = webserv

CFLAGS = -Wextra -Wall -Werror -std=c++98 -g3 #-fsanitize=address -g3
CC = c++

SRC = server.cpp												\
			../request/request.cpp								\
			../request/Sessions.cpp								\
			../request/SessionsList.cpp							\
			../request/requestCgi.cpp							\
			../response/Response.cpp							\
			../config_file/config_file.cpp						\
			../config_file/listen.cpp						\
			../config_file/server_name.cpp					\
			../config_file/error_page.cpp Record.cpp		\
			../response/CgiHandler.cpp

SRC_H = server.hpp Record.hpp				\
			../request/SessionsList.hpp		\
			../request/Sessions.hpp			\
			../response/CgiHandler.hpp		\
			../config_file/config_file.hpp

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
