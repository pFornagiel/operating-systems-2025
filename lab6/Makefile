CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb

all: first second third

first: first.o
	$(CC) first.o -o first

first.o: first.c
	$(CC) $(CFLAGS) first.c

second: second.o
	$(CC) second.o -o second

second.o: second.c
	$(CC) $(CFLAGS) second.c

third: third.o
	$(CC) third.o -o third

third.o: third.c
	$(CC) $(CFLAGS) third.c

.PHONY: clean all

NAME ?= 0

clean:
	rm -f *o first second third
