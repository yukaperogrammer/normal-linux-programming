#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define CURRENT_DIR  "."
#define PARENT_DIR   ".."
#define DELIMITER_STRING "/\0"

static void traverse(char *);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
		exit(EXIT_FAILURE);
	}


	/* begin traverse */
	traverse(argv[1]);
	
	return 0;
}

static void traverse(char *path)
{
	struct stat st;

	if (lstat(path, &st) < 0)
	{
		perror(path);
		return;
	}

	/* path is symlink */
	if (S_ISLNK(st.st_mode))
	{
		return;
	}

	/* path is file */
	if (S_ISREG(st.st_mode))
	{
		/* show file path and return */
		printf("%s\n", path);
		return;
	}

	/* path is diretory */
	DIR *dp;
	struct dirent *ent;

	dp = opendir(path);
	if (!dp)
	{
		/* can't open dirctory */
		perror(path);
		return;
	}
	
	size_t path_size = strlen(path);
	while ((ent = readdir(dp)))
	{
		if (strcmp(ent->d_name, CURRENT_DIR) == 0)
		{
			/* ignore current directory */
			continue;
		}

		if (strcmp(ent->d_name, PARENT_DIR) == 0)
		{
			/* ignore parent directory */
			continue;
		}

		/* traverse next directory */
		char *ptr = (char *)malloc(path_size + strlen(ent->d_name) + 2);
		if (!ptr)
		{
			fprintf(stderr, "%s: memory allocation error\n", path);
			continue;
		}
		strcpy(ptr, path);
		strcat(ptr, DELIMITER_STRING);
		strcat(ptr, ent->d_name);
		traverse(ptr);
		free(ptr);
	}

	/* current directory complite traverse */
	closedir(dp);
	return;
}

