CC = gcc
EXEC = myshell
OBJS = myshell.o error.o

$(EXEC): $(OBJS)
		$(CC) -o $(EXEC) $(OBJS)
myshell.o:myShell.c shell.h
		$(CC) -c $<
error.o:error.c error.h
		$(CC) -c $<

clean:
		rm -rf $(EXEC) *.o
