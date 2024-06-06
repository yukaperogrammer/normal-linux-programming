#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "no arguments\n");
		exit(EXIT_FAILURE);
	}

	for (i = 1; i < argc; i++) 
	{
		if (mkdir(argv[i], 0777) < 0)
		{
			perror(argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}

