#include <stdio.h>
#include <stdlib.h>

static void do_head(FILE *, long);

int main(int argc, char *argv[]) {
	long n;

	if (argc < 2) {
		fprintf(stderr, "%s: line number not given\n", argv[0]);
		exit(1);
	} else if (argc > 2) {
		fprintf(stderr, "%s: too arguments\n", argv[0]);
		exit(1);
	}

	n = atol(argv[1]);
	if (!n) {
		fprintf(stderr, "%s: given argument is not integer\n", argv[0]);
		exit(1);
	}
	do_head(stdin, n);

	return 0;
}

static void do_head(FILE *fp, long lines) {
	int c;

	while ((c = fgetc(stdin)) != EOF) {
		if (fputc(c, stdout) < 0) {
			perror("stdin");
			exit(1);
		}
		if (c == '\n') lines--;
		if (lines <= 0) break;
	}
}

