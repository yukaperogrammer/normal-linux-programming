#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	int mode;
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s mode filepath\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	mode = strtol(argv[1], NULL, 8);
	for (i = 2; i < argc; i++)
	{
		if (chmod(argv[i], mode) < 0)
		{
			perror(argv[2]);
		}
	}

	return 0;
}

