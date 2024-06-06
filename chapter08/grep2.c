#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

static void do_grep(regex_t *, FILE *);

int main(int argc, char *argv[]) {
	regex_t pat;
	int err;
	int i;
	char buf[1024];
	FILE *fp;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s pattern [file file ...]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	err = regcomp(&pat, argv[1], REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
	if (err != 0) {
		regerror(err, &pat, buf, sizeof(buf));
		fprintf(stderr, "%s", buf);
		exit(EXIT_FAILURE);
	}

	if (argc == 2) {
		do_grep(&pat, stdin);
	} else {
		for (i = 2; i < argc; i++) {
			fp = fopen(argv[i], "r");
			if (!fp) {
				perror(argv[i]);
				exit(EXIT_FAILURE);
			}

			do_grep(&pat, fp);

			if (fclose(fp) < 0) {
				perror(argv[i]);
				exit(EXIT_FAILURE);
			}
		}
	}
	regfree(&pat);

	return 0;
}

static void do_grep(regex_t *pat, FILE *fp) {
	char buf[4096];

	while(fgets(buf, sizeof(buf), fp)) {
		if (regexec(pat, buf, 0, NULL, 0) == 0) {
			printf("%s\n", buf);
		}
	}
}

