CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb

all: first catcher sender

first: first.o
	$(CC) first.o -o first

first.o: first.c
	$(CC) $(CFLAGS) first.c

catcher: catcher.o
	$(CC) catcher.o -o catcher

catcher.o: catcher.c
	$(CC) $(CFLAGS) catcher.c
	
sender: sender.o
	$(CC) sender.o -o sender

sender.o: sender.c
	$(CC) $(CFLAGS) sender.c

.PHONY: clean all

clean:
	rm -f *o catcher sender first
