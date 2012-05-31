#ifndef PARSER_H
#define PARSER_H

int parseInput(const char *input,
				char	*filename,
				char	**arguments,
				int		*isBackground,
				char	*outputRedirectionFilename,
				char	*inputRedirectionFilename);


#endif	// PARSER_H

