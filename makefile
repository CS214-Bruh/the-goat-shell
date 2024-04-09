CC = gcc
CFLAGS = -Wall -fsanitize=address -std=c99
#DEBUG = -DDEBUG

mysh: mysh.o
	$(CC) $(CFLAGS) $(DEBUG) mysh.o -o mysh

mysh.o: mysh.c
	$(CC) $(CFLAGS) $(DEBUG) -c -Wall mysh.c

clean:
	rm *.o ./mysh