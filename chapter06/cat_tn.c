#include <stdio.h>
#include <stdlib.h>

static void do_cat(const char *);
static void die(const char *);

int main(int argc, char *argv[]) {
	int i;

	if (argc < 2) {
		fprintf(stderr, "%s: file name not given\n", argv[0]);
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
		switch (c) {
			case '\t':
				if (fputs("\\t", stdout) < 0) die(path);
				break;
			case '\n':
				if (fputs("$\n", stdout) < 0) die(path);
				break;
			default:
				if (putchar(c) < 0) die(path);
				break;
		}
	}

	if (fclose(fp) < 0) die(path);
}

static void die(const char *s) {
	perror(s);
	exit(1);
}

