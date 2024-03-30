CC = gcc
CFLAGS = -Wall -fsanitize=address -std=c99
# DEBUG = -DDEBUG

mysh: mysh.o
	$(CC) $(CFLAGS) $(DEBUG) spchk.o -o mysh

mysh.o: mysh.c
	$(CC) $(CFLAGS) $(DEBUG) -c -Wall mysh.c
	