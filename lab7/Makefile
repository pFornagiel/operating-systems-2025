CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb 

all: server client

server: server.o
	$(CC) server.o -o server -lm

server.o: server.c
	$(CC) $(CFLAGS) server.c

client: client.o
	$(CC) client.o -o client -lm

client.o: client.c
	$(CC) $(CFLAGS) client.c

.PHONY: clean all

clean:
	rm -f *o client server
