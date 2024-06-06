#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#define BUFSIZE 1024 

static void do_ls(char *);

int main(int argc, char *argv[])
{
	int i;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <dir> <dir> ...\n", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		do_ls(argv[i]);
		printf("\n");
	}

	return 0;
}

static void do_ls(char *path)
{
	DIR *dp;
	struct dirent *ent;
	struct stat st;
	struct passwd *pw;
	char filepath[BUFSIZE];
	char *ptr;
	int i;

	dp = opendir(path);
	if (!dp) {
		perror(path);
		exit(1);
	}

	strncpy(filepath, path, sizeof(filepath));
	ptr = strchr(filepath, '\0');
	if (!ptr) {
		exit(1);
	}
	*ptr++ = '/';

	while ((ent = readdir(dp))) {
		/* initialize filepath */
		for (i = 0; i < (BUFSIZE - (ptr - filepath)); i++) {
			ptr[i] = '\0';
		}

		/* generate path */
		strncpy(ptr, ent->d_name, (BUFSIZE - (ptr - filepath)));

		/* lstat */
		if (lstat(filepath, &st) < 0) {
			perror(filepath);
			exit(1);
		}

		pw = getpwuid(st.st_uid);

		/* print info */
		printf("%s\n", ent->d_name);
		printf("%s\t%s\n", pw->pw_name, ctime(&st.st_ctim.tv_sec));
	}

	if (closedir(dp) < 0) {
		fprintf(stderr, "error close directory\n");
		exit(1);
	}
}

