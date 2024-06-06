#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

static void do_ls(char *);

int main(int argc, char *argv[])
{
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s [directory ...]", argv[0]);
		exit(EXIT_FAILURE);
	}

	for (i = 1; i < argc; i++)
	{
		do_ls(argv[i]); 
	}

	return 0;
}

static void do_ls(char *dirname)
{
	DIR *dp;
	struct dirent *ent;

	dp = opendir(dirname);
	if (!dp)
	{
		perror(dirname);
		exit(EXIT_FAILURE);
	}

	while ((ent = readdir(dp)))
	{
		printf("%s\n", ent->d_name);
	}	

	if (closedir(dp) < 0)
	{
		perror(dirname);
		exit(EXIT_FAILURE);
	}
}

