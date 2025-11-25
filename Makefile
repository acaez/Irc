CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g
RM = rm -rf

SRC_DIR = srcs/
INCS_DIR = incs/

SRC =	main.cpp\
		Server.cpp\
		Client.cpp\
		Fd.cpp\
		Channel.cpp\
		Command.cpp\
		Reply.cpp\
		Utils.cpp\

SRC_PATH = $(addprefix $(SRC_DIR), $(SRC))

OBJS = $(SRC_PATH:.cpp=.o)

%.o: %.cpp
	@$(CPP) $(CPPFLAGS) -c $< -o $@

NAME = ircserv

RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
PURPLE = \033[35m
WHITE = \033[37m

all: $(NAME)

sanitize: $(OBJS)
	@$(CPP) $(CPPFLAGS) -fsanitize=address,undefined -I $(INCS_DIR) $(SRC_PATH) -o $(NAME)
	@echo "$(NAME) created with flag '-fsanitize=address,undefined'"

valgrind: $(OBJS)
	@$(CPP) $(CPPFLAGS) -g -O0 -I $(INCS_DIR) $(SRC_PATH) -o $(NAME)
	@echo "$(NAME) created with flag '-g -O0'"

debug: $(OBJS)
	@$(CPP) $(CPPFLAGS) -g -I $(INCS_DIR) $(SRC_PATH) -o $(NAME)
	@echo "$(NAME) created with flag '-g'"

$(NAME): $(OBJS)
	@$(CPP) $(CPPFLAGS) -I $(INCS_DIR) $(SRC_PATH) -o $(NAME)
	@echo "$(GREEN)$(NAME) created\n$(WHITE)./ircserv$(BLUE) <port> <password> $(WHITE)"

clean:
	@$(RM) $(OBJS)
	@echo "$(YELLOW).o files deleted$(WHITE)"
	@$(RM) $(NAME).dSYM

fclean: clean
	@$(RM) $(NAME)
	@echo "$(PURPLE)$(NAME) executable deleted$(WHITE)"

re: fclean all

.PHONY:
	re all clean fclean valgrind sanitize