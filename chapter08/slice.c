#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>

#define BUF_SIZE 1024
#define ERR_BUF_SIZE 1024

static void do_slice(regex_t *, FILE *);

int main(int argc, char *argv[])
{
	int i;
	int err;
	char err_buf[ERR_BUF_SIZE];
	regex_t pat;
	FILE *fp;
	
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s pattern [file...]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	err = regcomp(&pat, argv[1], REG_EXTENDED | REG_NEWLINE);
	if (err != 0)
	{
		regerror(err, &pat, err_buf, sizeof(err_buf));
		fprintf(stderr, "%s\n", err_buf);
		exit(EXIT_FAILURE);
	}

	if (argc == 2)
	{
		do_slice(&pat, stdin);
	}
	else
	{
		for (i = 2; i < argc; i++)
		{
			fp = fopen(argv[i], "r");
			if (!fp)
			{
				perror(argv[i]);
				regfree(&pat);
				exit(EXIT_FAILURE);
			}

			do_slice(&pat, fp);

			if (fclose(fp) < 0)
			{
				perror(argv[i]);
				regfree(&pat);
				exit(EXIT_FAILURE);
			}
		}
	}

	regfree(&pat);

	return 0;
}

static void do_slice(regex_t *pat, FILE *fp)
{
	char buf[BUF_SIZE];

	while (fgets(buf, sizeof(buf), fp))
	{
		regmatch_t matched[1];
		if (regexec(pat, buf, 1, matched, 0) == 0)
		{
			char *str = buf + matched[0].rm_so;
			regoff_t len = matched[0].rm_eo - matched[0].rm_so;
			fwrite(str, len, 1, stdout);
			fputc('\n', stdout);
		}
	}
}

