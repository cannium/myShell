#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "shell.h"

void printError(char* errorString)
{
	fprintf(stderr, "%s\n", errorString);
}

void printErrorAndQuit(char* errorString)
{
	printError(errorString);
	exit(-1);
}
