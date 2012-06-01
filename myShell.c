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
static int runBuiltinCommand(char *command[]);
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

		if( !runBuiltinCommand(arguments))
		{
			pid_t pid;
			if( (pid = fork()) < 0)
				printErrorAndQuit("fork error");

			else if(pid == 0)
			{   /* child */
				if(isBackground)
				{
					// ignore interruption if run in background
					if (signal(SIGINT, SIG_IGN) == SIG_ERR)
						printErrorAndQuit("signal error");
				}
				else
				{					
					if (signal(SIGINT, childInterruptionHandler) == SIG_ERR)
						printErrorAndQuit("signal error");
				}

				// if output redirection exists:
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
					close(outfd);
				}
				// if input redirection exists:
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
					close(infd);
				}
				execvp(arguments[0], arguments);
				printErrorAndQuit("couldn't execute");
			}

			/* parent */
			if(isBackground)
			{
				backgroundJob[pid] = 1;
				printf("%s [%d]\n", arguments[0], pid);
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
	int i;

	// check if background job ends and waitpid() for it
	for(i = 0; i < MAX_BACKGROUND_JOB; i++)
	{
		if(backgroundJob[i])
		{
			if(i == (waitpid(i, &status, WNOHANG)))
			{
				backgroundJob[i] = 0;
				printf("[%d] done\n", i);
			}
		}

	}
}

void printPrompt()
/*
 *	print prompt according to environment variable $PE
 *	if $PE not exists, print default prompt "%"
 */
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
						// if \u appears, print current user name
						printf("%s", getlogin());
						break;
					case 'w':
						// if \w appears, print current working directory
						getcwd(buffer, BUFFER_SIZE);
						printf("%s", buffer);
						break;
					case 'h':
						// if \h appears, print host name
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
	}

	fflush(NULL);	
}

int runBuiltinCommand(char **command)
/*
 *	try run (char **)command as builtin command.
 *	if successful, return TRUE,
 *	if failed, return FALSE
 */
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
		char *name, *value;
		name = strtok(command[1], "=");
		value = strtok(NULL, "=");

		if(name == NULL || value == NULL)
		{
			printError("set environment variable failed");
			return TRUE;
		}

		if(DEBUG)
			printf("name %s, value %s\n", name, value);

		if( setenv(name, value, TRUE) != 0)
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
