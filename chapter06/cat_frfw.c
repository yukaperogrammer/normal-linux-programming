#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 2048

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
	FILE *fp;
	char buf[BUFFER_SIZE];
	size_t n_read;
	size_t n_write;

	fp = fopen(path, "r");
	if (!fp) die(path);

	while (1) {
		n_read = fread(buf, sizeof(char), BUFFER_SIZE, fp);
		if (ferror(fp)) die(path);

		n_write = fwrite(buf, sizeof(char), n_read, stdout);
		if (n_write < n_read) die(path);

		if (feof(fp)) break;
	}

	if (fclose(fp) < 0) die(path);
}

static void die(const char *s) {
	perror(s);
	exit(1);
}

