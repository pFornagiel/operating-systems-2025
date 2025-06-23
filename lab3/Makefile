CC=gcc
CFLAGS= -c -Wall -Wextra -ggdb

all: flipper

flipper: flipper.o
	$(CC) flipper.o -o flipper

flipper.o: flipper.c
	$(CC) $(CFLAGS) flipper.c

.PHONY: clean all

NAME ?= 0

clean:
	@if [ "$(NAME)" = "0" ]; then \
		rm -f *o flipper; \
	else \
		rm -f *o flipper; \
		rm -rf $(NAME); \
	fi