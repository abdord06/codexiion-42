NAME        = codexion

CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread
RM          = rm -f

SRCS        = main.c \
              time.c \
              heap.c \
			  init.c\
			  parse.c\
              arbitrator.c \
              routine.c \
              monitor.c \
              cleanup.c

OBJS        = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re