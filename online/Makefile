CC = gcc
CFLAGS = -g -w -Wall

OBJS = main.o       \
       itoa.o       \
       netipc.o     \
       multi.o      \
       string_net.o

NAME = online

all: $(OBJS)
	${CC} $(CFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -rf $(OBJS) $(NAME)

main.o: srcs/main.c includes/include.h
	${CC} $(CFLAGS) -c srcs/main.c
itoa.o: srcs/itoa.c includes/itoa.h
	${CC} $(CFLAGS) -c srcs/itoa.c
netipc.o: srcs/netipc.c includes/netipc.h
	${CC} $(CFLAGS) -c srcs/netipc.c
multi.o: srcs/multi.c includes/multi.h
	${CC} $(CFLAGS) -c srcs/multi.c
string_net.o: srcs/string_net.c includes/string_net.h
	${CC} $(CFLAGS) -c srcs/string_net.c