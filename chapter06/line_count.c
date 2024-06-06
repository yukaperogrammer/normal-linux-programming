#include <stdio.h>
#include <stdlib.h>

static void do_count(const char *);
static void print_count(const char *, int);
static void die(const char *);

int main(int argc, char *argv[]) {
	int i;

	if (argc < 2) {
		fprintf(stderr, "%s: file name not given\n", argv[0]);\
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		do_count(argv[i]);
	}

	return 0;
}

static void do_count(const char *path) {
	int c;
	int old_c = '\n';
	int lines = 0;
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp) die(path);

	while ((c = fgetc(fp)) != EOF) {
		old_c = c;
		if (c == '\n') lines++;
	}

	if (fclose(fp) < 0) die(path);

	if (old_c != '\n') lines++;

	print_count(path, lines);
}

static void print_count(const char *path, int lines) {
	printf("%s: %d lines.\n", path, lines);
}

static void die(const char *s) {
	perror(s);
	exit(1);
}

