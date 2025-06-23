CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb

all: first second

first: first.o
	$(CC) first.o -o first

first.o: first.c
	$(CC) $(CFLAGS) first.c

second: second.o
	$(CC) second.o -o second

second.o: second.c
	$(CC) $(CFLAGS) second.c

.PHONY: clean all

clean:
	rm -f *o first second 
