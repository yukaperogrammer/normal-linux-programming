#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <filename> <new filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	rename(argv[1], argv[2]);

	close(fd);

	return 0;
}
	
