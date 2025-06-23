CC=gcc
CFLAGS= -c -Wall -Wextra -pedantic -ggdb
FPIC= -fpic
SHAREDFLAGS = -L. -lcollatz -Wl,-rpath=.

static: collatz_static
shared: collatz_shared

all: main_dynamic main_static main_shared


# LIBRARY COMPILATION
collatz_static: collatz.o
	$(CC) $(CFLAGS) collatz.c -o collatz.o
	ar rcs libcollatz.a collatz.o

collatz_shared: collatz.o
	$(CC) $(CFLAGS) $(FPIC) collatz.c -o collatz.o
	gcc -shared -o libcollatz.so collatz.o


# STATIC MAIN COMPILATION
main_static: main_static.o
	$(CC) main_static.o libcollatz.a -o main_static

main_static.o: main_static.c
	$(CC) $(CFLAGS) main_static.c


# SHARED MAIN COMPILATION
main_shared: main_shared.o
	$(CC) main_shared.o $(SHAREDFLAGS) -o main_shared

main_shared.o: main_shared.c
	$(CC) $(CFLAGS) main_shared.c


# DYNAMIC MAIN COMPILATION
main_dynamic: main_dynamic.o
	$(CC) main_dynamic.o  -o main_dynamic

main_dynamic.o: main_dynamic.c
	$(CC) -DDYNAMICVARIANT $(CFLAGS) main_dynamic.c


.PHONY: clean
clean:
	rm -f *.o *.so *.a main_static main_dynamic main_shared 