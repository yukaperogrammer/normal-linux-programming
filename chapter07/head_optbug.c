#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE
#include <getopt.h>

static void do_head(FILE *, long);

#define DEFAULT_N_LINES 10
static struct option longopts[] = {
	{"lines", required_argument, NULL, 'n'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
	int opt;
	long nlines = DEFAULT_N_LINES;

	while ((opt = getopt_long(argc, argv, "n", longopts, NULL)) != -1) {
		switch (opt) {
			case 'n':
				nlines = atol(optarg);
				break;
			case 'h':
				fprintf(stdout, "Usage: %s [-n LINES] [FILE...]\n", argv[0]);
				exit(0);
			case '?':
				fprintf(stderr, "Usage: %s [-n LINES] [FILE...]\n", argv[0]);
				exit(1);
		}
	}

	if (optind == argc) {
		do_head(stdin, nlines);
	} else {
		int i;
		for (i = optind; i < argc; i++) {
			FILE *fp;

			fp = fopen(argv[i], "r");
			if (!fp) {
				perror(argv[i]);
				exit(1);
			}

			do_head(fp, nlines);
			if (fclose(fp) < 0) {
				perror(argv[i]);
				exit(1);
			}
		}
	}
	return 0;
}


static void do_head(FILE *fp, long nlines) {
	int c = '\n';

	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n') nlines--;
		if (putchar(c) < 0) return;
		if (nlines <= 0) break;
	}
}

