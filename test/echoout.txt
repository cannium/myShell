#include <stdio.h>

int main()
{
	char buffer[1024];
	while( fgets(buffer, 1024, stdin) != NULL)
	{
		fputs(buffer, stdout);
	}
}
