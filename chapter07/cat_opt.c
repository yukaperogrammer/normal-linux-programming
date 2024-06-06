#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE
#include <getopt.h>

static int is_visible = 0;
static struct option longopts[] = {
	{"visible", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};
static void do_cat(FILE *);

int main(int argc, char *argv[]) {
	int opt;
	int i;
	FILE *fp;

	while((opt = getopt_long(argc, argv, "v", longopts, NULL)) != -1) {
		switch (opt) {
			case 'v':
				is_visible = 1;
				break;
			case '?': /* fall through */
			default:
				fprintf(stderr, "Usage: %s [-v or --visible] [file file ...]\n", argv[0]);
				exit(1);
		}
	}

	if (optind == argc) {
		do_cat(stdin);
	} else {
		for (i = optind; i < argc; i++) {
			fp = fopen(argv[i], "r");
			if (!fp) exit(1);

			do_cat(fp);

			if (fclose(fp) < 0) exit(1);
		}
	}

	return 0;
}


static void do_cat(FILE *fp) {
	int c;

	while ((c = fgetc(fp)) != EOF) {
		if (c == '\t' && is_visible) {
			if(fputs("\\t", stdout) < 0) exit(1);
			continue;
		}

		if (c == '\n' && is_visible) {
			if (fputs("\\n", stdout) < 0) exit(1);
		}

		if (putchar(c) < 0) exit(1);
	}
}

