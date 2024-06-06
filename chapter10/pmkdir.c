#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define DIRECTORY_SEPARATOR '/'
#define MODE 0777
#define TRUE 1
#define FALSE 0

static void do_mkdir(char *, int);

int main(int argc, char *argv[])
{
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <new dir path...>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	for (i = 1; i < argc; i++)
	{
		errno = 0;
		do_mkdir(argv[i], TRUE);
		if (errno != 0) perror(argv[i]);
	}

	return 0;
}

static void do_mkdir(char *path, int is_first)
{
	char *ptr = NULL;
	char *parent_directory = NULL;
	int separator_ind;

	if (mkdir(path, MODE) < 0)
	{
		switch (errno)
		{
			case EEXIST:
				if (!is_first) errno = 0;
				return;
			case ENOENT:
				errno = 0;
				break;
			default:
				break;
		}
	} else {
		return;
	}

	ptr = strrchr(path, DIRECTORY_SEPARATOR);
	separator_ind = ptr - path;

	parent_directory = (char *)malloc(separator_ind + 1);
	if (!parent_directory)
	{
		/* memory allocation error */
		return;
	}

	strncpy(parent_directory, path, separator_ind);
	parent_directory[separator_ind] = '\0';

	/* make parent directory*/
	do_mkdir(parent_directory, FALSE);

	/* errors */
	if (errno != 0)
	{
		switch (errno)
		{
			case EEXIST:
				break;
			default:
				free(parent_directory);
				return;
		}
	}

	/* make current directory */
	if (mkdir(path, MODE) < 0)
	{
		free(parent_directory);
		return;
	}

	free(parent_directory);
	return;
}

