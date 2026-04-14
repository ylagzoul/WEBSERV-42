RS_SRCFL =  main.cpp Response.cpp
CF_SRCFL = \
			../config_file/config_file.cpp \
			../config_file/config_file.cpp \
			../config_file/Helpful_functions/get_set_conf.cpp \
			../config_file/Helpful_functions/ft_ft.cpp \
			../config_file/line/listen.cpp \
			../config_file/line/server_name.cpp \
			../config_file/line/error_page.cpp

HEADERS = Response.hpp ../config_file/config_file.hpp

SRCFL = $(RS_SRCFL) $(CF_SRCFL)
OBJFL = $(SRCFL:.cpp=.o)

NAME = mini_res
CPP = c++
RM = rm -rf
FLAGS = -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJFL)
	$(CPP) $(FLAGS) $^ -o $@

%.o: %.cpp $(HEADERS)
	$(CPP) -c $(FLAGS) $< -o $@

clean:
	$(RM) $(OBJFL)

fclean: clean
	$(RM) $(NAME)

re: fclean all
