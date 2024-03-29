#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "parser.h"


int parseInput( char	*input,
				char	**arguments,
				int		*isBackground,
				char	*outputRedirectionFilename,
				char	*inputRedirectionFilename)
/*
 *	parse the input and figure out the parameters.
 *	return -1 if error, 0 if OK
 */
{
	char buffer[BUFFER_SIZE];
	char *tokenPointer;
	int argumentsCount = 0;
	int inputRedirectionOccurrence = 0;
	int outputRedirectionOccurrence = 0;

	tokenPointer = strtok(input, " ");
	while(tokenPointer != NULL)
	{
		sscanf(tokenPointer, "%s", buffer);

		if( strcmp(buffer, "<") == 0)
		{
			// input redirection can only appear once
			if(inputRedirectionOccurrence >= 1)
				return -1;
			inputRedirectionOccurrence++;

			tokenPointer = strtok(NULL, " ");
			if(tokenPointer == NULL)
				return -1;
			else
			{
				sscanf(tokenPointer, "%s", inputRedirectionFilename);
				tokenPointer = strtok(NULL, " ");
				continue;
			}
		}

		if( strcmp(buffer, ">") == 0)
		{
			// output redirection can only appear once
			if(outputRedirectionOccurrence >= 1)
				return -1;
			outputRedirectionOccurrence++;

			tokenPointer = strtok(NULL, " ");
			if(tokenPointer == NULL)
				return -1;
			else
			{
				sscanf(tokenPointer, "%s", outputRedirectionFilename);
				tokenPointer = strtok(NULL, " ");
				continue;
			}
		}

		if( strcmp(buffer, "&") == 0)
		{
			tokenPointer = strtok(NULL, " ");

			// there should be no more input after "&"
			if(tokenPointer != NULL)
				return -1;	
			else
			{
				*isBackground = TRUE;
				return 0;
			}
		}

		int len = strlen(buffer) + 1;
		char *temp = (char *) malloc( len * sizeof(char));
		strncpy(temp, buffer, len);
		arguments[argumentsCount++] = temp;

		tokenPointer = strtok(NULL, " ");
	}
	return 0;
}

