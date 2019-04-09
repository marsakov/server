NAME = server
SRCS = server.c
FLAGS = -lpthread
OBJ = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@ gcc $(FLAGS) $(SRCS) -o $(NAME) 

%.o:%.c
	@ gcc -o $@ -c $< 

clean:
	@ rm -f $(OBJ)

fclean: clean
	@ rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
