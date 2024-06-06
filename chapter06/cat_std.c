#include <stdio.h>
#include <stdlib.h>


static void do_cat(const char *);
static void die(const char *);

int main(int argc, char *argv[]) {
	int i;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [filename] [filename] ... ", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		do_cat(argv[i]);
	}

	return 0;
}


static void do_cat(const char *path) {
	int c;
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp) die(path);

	while ((c = fgetc(fp)) != EOF) {
		if (fputc(c, stdout) < 0) die(path);
	}

	if (fclose(fp) < 0) die(path);
}


static void die(const char *s) {
	perror(s);
	exit(1);
}

