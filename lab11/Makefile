CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb

all: server client

server: server.o cprint.o
	$(CC) server.o cprint.o -o server

server.o: server.c
	$(CC) $(CFLAGS) server.c

client: client.o cprint.o
	$(CC) client.o cprint.o -o client

client.o: client.c
	$(CC) $(CFLAGS) client.c

cprint.o: cprint.c cprint.h
	$(CC) $(CFLAGS) cprint.c

clean:
	rm -f *.o main server client
