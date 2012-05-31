#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "shell.h"
#include "error.h"

static void parentInterruptionHandler(int);
static void childInterruptionHandler(int);
static void sigchildHandler(int);
static void printPrompt();
static int runBuiltInCommand(char *command[]);
static void setSignalHandler();

// hash table, if pid is a background job, 
// record it by set backgroundJob[pid] = 1
int backgroundJob[MAX_BACKGROUND_JOB] = {0};


int main(void)
{
    char buffer[BUFFER_SIZE];

	setSignalHandler();

	printf("welcome, %s!\n", getlogin());

    while(1) 
	{
		printPrompt();
		if( fgets(buffer, BUFFER_SIZE, stdin) == NULL)
			break;

        buffer[strlen(buffer) - 1] = 0;    /* replace newline with null */
		if( strlen(buffer) == 0)
			continue;

		char *arguments[BUFFER_SIZE] = {0};
		int isBackground = FALSE;
		char outputRedirectionFilename[BUFFER_SIZE] = {0};
		char inputRedirectionFilename[BUFFER_SIZE] = {0};
		int error = parseInput(buffer, arguments, &isBackground,
				outputRedirectionFilename, inputRedirectionFilename);
		if(error)
		{
			printError("input parse error");
			continue;
		}

		if(DEBUG)
		{
			char **p = arguments;
			while( *p != NULL)
			{
				printf("argument: %s\n", *p);
				p++;
			}
			printf("isBackground: %s\n", isBackground ? "TRUE":"FALSE");
			printf("outputRedirectionFilename: %s\n", 
					outputRedirectionFilename);
			printf("inputRedirectionFilename: %s\n",
					inputRedirectionFilename);
			printf("\n");
		}

		if( !runBuiltInCommand(arguments))
		{
			pid_t pid;
			if( (pid = fork()) < 0)
				printErrorAndQuit("fork error");

			else if(pid == 0)
			{   /* child */
				if (signal(SIGINT, childInterruptionHandler) == SIG_ERR)
					printErrorAndQuit("signal error");

				if(outputRedirectionFilename[0] != 0)
				{
					int outfd;
					if( (outfd = open(outputRedirectionFilename, O_WRONLY |
								O_CREAT | O_TRUNC, FILE_MODE)) < 0)
					{
						printError("can't open file for output");
						exit(-1);
					}
					dup2(outfd, fileno(stdout));
				}
				if(inputRedirectionFilename[0] != 0)
				{
					int infd;
					if( (infd = open(inputRedirectionFilename, O_RDONLY))
						   	< 0)
					{
						printError("can't open file for input");
						exit(-1);
					}
					dup2(infd, fileno(stdin));
				}
				execvp(arguments[0], arguments);
				printErrorAndQuit("couldn't execute");
			}

			/* parent */
			if(isBackground)
			{
				backgroundJob[pid] = 1;
				printf("%s [%d]\n", pid, arguments[0]);
			}
			else
			{
				int status;	
				if( (pid = waitpid(pid, &status, 0)) < 0)
					printErrorAndQuit("waitpid error");
			}
		}

		// cleanup because of malloc in parser
		char **p = arguments;
		while( *p != NULL)
		{
			free(*p);
			*p = NULL;
			p++;
		}
    }
    exit(0);
}

void parentInterruptionHandler(int signo)
{
	printf("\n");
	printPrompt();
	fflush(NULL);
}

void childInterruptionHandler(int signo)
{
	printf("\ninterrupted\n");
	exit(0);
}

void sigchildHandler(int signo)
{
	pid_t pid;
	int status;
	while( (pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		backgroundJob[pid] = 0;
		printf("[%d] done\n", pid);
	}
}

void printPrompt()
{
	char *promptEnvironment = getenv("$PE");
	if( promptEnvironment == NULL)
		printf("%% ");
	else
	{
		int len = strlen(promptEnvironment);
		int i;
		char buffer[BUFFER_SIZE];
		for(i = 0; i < len; i++)
		{
			if(promptEnvironment[i] == '\\')
			{
				switch(promptEnvironment[++i])
				{
					case 'u':
						printf("%s", getlogin());
						break;
					case 'w':
						getcwd(buffer, BUFFER_SIZE);
						printf("%s", buffer);
						break;
					case 'h':
						gethostname(buffer, BUFFER_SIZE);
						printf("%s", buffer);
						break;
					default:
						printf("%c", promptEnvironment[i]);
				}
			}
			else
				printf("%c", promptEnvironment[i]);
		}
		printf(" ");
		fflush(NULL);
	}
}

int runBuiltInCommand(char **command)
{
	if( strcmp(command[0], "cd") == 0)
	{
		if( chdir(command[1]) != 0)
			printError("change directory failed");
	}
	else if( strcmp(command[0], "exit") == 0)
	{
		printf("bye!\n");
		exit(0);
	}
	else if( strcmp(command[0], "export") == 0)
	{
		if( putenv(command[1]) != 0)
			printError("set environment variable failed");
	}
	else if( strcmp(command[0], "echo") == 0)
	{
		printf("%s\n", getenv(command[1]));
	}
	else 
		return FALSE;

	return TRUE;
}

void setSignalHandler()
{
	if( signal(SIGINT, parentInterruptionHandler) == SIG_ERR)
		printErrorAndQuit("signal error");

	if( signal(SIGCHLD, sigchildHandler) == SIG_ERR)
		printErrorAndQuit("signal error");
}
