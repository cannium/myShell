#ifndef SHELL_H
#define SHELL_H


#define BUFFER_SIZE 4096

#define TRUE	1
#define FALSE	0


static void sig_int(int);        /* our signal-catching function */
static void printPrompt();


#endif /* SHELL_H */
