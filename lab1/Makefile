CC=gcc
CFLAGS= -c -Wall -Wextra -pedantic -ggdb

all: countdown

countdown: countdown.o
	$(CC) countdown.o -o countdown

countdown.o: countdown.c
	$(CC) $(CFLAGS) countdown.c

.PHONY: clean all
clean:
	rm -f *o countdown