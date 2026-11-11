CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -g

all: mysh

mysh: mysh.o execute.o
	$(CC) $(CFLAGS) -o $@ $^

mysh.o: mysh.c mysh.h
	$(CC) $(CFLAGS) -c mysh.c

execute.o: execute.c mysh.h
	$(CC) $(CFLAGS) -c execute.c

clean:
	rm -f mysh *.o

.PHONY: all clean
