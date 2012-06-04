CC = gcc
EXEC = myshell
OBJS = myShell.o error.o parser.o

$(EXEC): $(OBJS)
		$(CC) -o $(EXEC) $(OBJS)
myShell.o:myShell.c shell.h
		$(CC) -c $<
error.o:error.c error.h
		$(CC) -c $<
parser.o:parser.c parser.h
		$(CC) -c $<

clean:
		rm -rf $(EXEC) *.o
