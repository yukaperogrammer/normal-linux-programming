#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE
#include <getopt.h>

#define DEFAULT_N_LINES 10
#define BUFFER_SIZE 256

static struct option longopts[] = {
	{"lines", required_argument, NULL, 'n'},
	{0, 0, 0, 0}
};
static void do_tail(FILE *, int);
static void do_print(char **, int, int);
static void memory_free(char **, int);

int main(int argc, char *argv[]) {
	int i;
	int opt;
	int nlines = DEFAULT_N_LINES;
	FILE *fp;

	while((opt = getopt_long(argc, argv, "n:", longopts, NULL)) != -1) {
		switch (opt) {
			case 'n':
				nlines = atoi(optarg);
				if (nlines == 0) exit(EXIT_FAILURE);
				break;
			case '?': /* fall through */
			default:
				fprintf(stderr, "Usage: %s [-n LINES or --lines LINES] [file file ...]\n", argv[0]);
				exit(EXIT_FAILURE);
		}		
	}

	if (optind == argc) {
		do_tail(stdin, nlines);
	} else {
		for (i = optind; i < argc; i++) {
			fp = fopen(argv[i], "r");
			if (!fp) {
				perror(argv[i]);
				exit(EXIT_FAILURE);
			}

			do_tail(fp, nlines);

			if (fclose(fp) < 0) {
				perror(argv[i]);
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}

static void do_tail(FILE *fp, int nlines) {
	char buf[BUFFER_SIZE];
	char **ptr;
	int i = 0;
	int j = 0;
	int ring_buf_i = 0;

	/* ptr is array of pointer to char */
	ptr = (char **)calloc(nlines, sizeof(char *));
	if (!ptr) {
		fprintf(stderr, "memory allocation error");
		exit(EXIT_FAILURE);
	}

	/* ptr[i] is pointer to char */
	for (i = 0; i < nlines; i++) {
		ptr[i] = (char *)malloc(BUFFER_SIZE);

		/* memory allocation error, all memory free */
		if (!ptr[i]) {
			fprintf(stderr, "memory allocation error");
			memory_free(ptr, i);
			exit(EXIT_FAILURE);
		}
	}
	
	/* do_tail main process */
	for (;;) {
		fgets(buf, sizeof(buf), fp);
		if (ferror(fp)) {
			fprintf(stderr, "file reading error");

			/* file reading error, all memory free */
			memory_free(ptr, nlines);

			fclose(fp);
			exit(EXIT_FAILURE);
		}
		
		/* end of file */
		if (feof(fp)) break;

		/* copy string */
		for (j = 0; (j < BUFFER_SIZE - 1) && (buf[j] != '\0'); j++) {
			*(ptr[ring_buf_i] + j) = buf[j];
		}
		*(ptr[ring_buf_i] + j) = '\0';

		/* ring buffer index update */
		ring_buf_i = (ring_buf_i + 1) % nlines;
	}

	/* do print */
	do_print(ptr, ring_buf_i, nlines);

	/* all memory free */
	memory_free(ptr, nlines);
}

static void do_print(char **ptr, int begin, int num) {
	int i;
	int max = num;

	for(i = begin; num > 0 ; i++, num--) {
		if (i >= max) i = 0;
		printf("%s", ptr[i]);
	}
}

static void memory_free(char **ptr, int end) {
	int i;

	for(i = 0; i < end; i++) {
		free(ptr[i]);
	}
	free(ptr);
}

