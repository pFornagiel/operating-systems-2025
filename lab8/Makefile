CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb 

all: server client printer




server: server.o queue.o
	$(CC) server.o queue.o -o server -lm

server.o: server.c
	$(CC) $(CFLAGS) server.c

printer: printer.o queue.o
	$(CC) printer.o queue.o -o printer -lm

printer.o: printer.c 
	$(CC) $(CFLAGS) printer.c

client: client.o queue.o
	$(CC) client.o queue.o -o client -lm

client.o: client.c 
	$(CC) $(CFLAGS) client.c

queue.o: queue.c
	$(CC) $(CFLAGS) queue.c 

.PHONY: clean all

clean:
	rm -f *o client server printer
