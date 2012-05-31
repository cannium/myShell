#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"
#include "error.h"


int main(void)
{
    char buffer[BUFFER_SIZE];

    if (signal(SIGINT, sig_int) == SIG_ERR)
        printErrorAndQuit("signal error");

    while (1) 
	{
		printPrompt();
		fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer) - 1] = 0;    /* replace newline with null */

		char filename[BUFFER_SIZE] = {0};
		char *arguments[BUFFER_SIZE] = {0};
		int isBackground = FALSE;
		char outputRedirectionFilename[BUFFER_SIZE] = {0};
		char inputRedirectionFilename[BUFFER_SIZE] = {0};
		int error = parseInput(buffer, filename, arguments, &isBackground,
				outputRedirectionFilename, inputRedirectionFilename);
		if(error)
		{
			printError("input parse error");
			continue;
		}

		pid_t pid;
        if ( (pid = fork()) < 0)
            printErrorAndQuit("fork error");

        else if (pid == 0) 
		{   /* child */
            execlp(buffer, buffer, (char *) 0);
            printError("couldn't execute");
            exit(127);
        }

        /* parent */
	    int status;	
        if ( (pid = waitpid(pid, &status, 0)) < 0)
            printErrorAndQuit("waitpid error");
        
    }
    exit(0);
}

void sig_int(int signo)
{
    printf("interrupt\n%% ");
}

void printPrompt()
{
	printf("%% ");
}
