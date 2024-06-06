#include <stdio.h>
#include <stdlib.h>

static void do_head(FILE *, const char *, long);
static void die(const char *);
static void print_how_to(const char *);

int main(int argc, char *argv[]) {
	int i;
	long nlines;
	FILE *fp;

	if (argc < 2) {
		print_how_to(argv[0]);
	}

	if ((nlines = atol(argv[1])) == 0) {
		print_how_to(argv[0]);
	}

	if (nlines < 0) {
		fprintf(stderr, "first option is positive integer only\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 2) {
		do_head(stdin, argv[1], nlines);
	} else {
		for (i = 2; i < argc; i++) {
			if ((fp = fopen(argv[i], "r")) == NULL) die(argv[i]);
			do_head(fp, argv[i], nlines);
			if (fclose(fp) < 0) die(argv[1]);
		}
	}

	return 0;
}

static void do_head(FILE *fp, const char *path, long nlines) {
	int c;

	while((c = fgetc(fp)) != EOF) {
		if (c == '\n') nlines--;
		if (putchar(c) < 0) die(path);
		if (nlines <= 0) break;
	}
}

static void die(const char *s) {
	perror(s);
	exit(EXIT_FAILURE);
}

static void print_how_to(const char *command) {
	fprintf(stderr, "Usage: %s n [file file ...]\n", command);
	exit(EXIT_FAILURE);
}

