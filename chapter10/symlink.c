#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s file/directory linkname\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (symlink(argv[1], argv[2]) < 0)
	{
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	return 0;
}

