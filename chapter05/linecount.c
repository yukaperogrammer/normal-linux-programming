#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 2048

static int line_count(const char *path);
static void die(const char *s);

int main(int argc, char *argv[]) {
	int i;
	int lcount;

	if (argc < 2) {
		fprintf(stdout, "%s:file path not given\n", argv[0]);
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		lcount = line_count(argv[i]);
		printf("%s: %d lines.\n", argv[i], lcount);
	}

	return 0;
}

static int line_count(const char *path) {
	int fd;
	char buf[BUFFER_SIZE];
	int n;
	int i;
	int lcount = 0;

	fd = open(path, O_RDONLY);
	if (fd < 0) die(path);

	for(;;) {
		n = read(fd, buf, sizeof(buf));
		if (n < 0) die(path);
		if ( n == 0) break;
		for(i = 0; i < n; i++) {
			if (buf[i] == '\n') lcount++;
		}
	}

	if (close(fd) < 0) die(path);

	return lcount;
}

static void die(const char *s) {
	perror(s);
	exit(1);
}

