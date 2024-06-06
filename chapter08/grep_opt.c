#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <ctype.h>

#define _GNU_SOURCE
#include <getopt.h>

#define BUFFER_SIZE 1024
#define ERR_BUFFER_SIZE 1024
#define IGNORE_UPPERLOWER 0x0001
#define WITHOUT_PATTERN   0x0002

unsigned int option_flag = 0;
static void do_grep(regex_t *, FILE *);
static char *lower_to_upper(char *);

int main(int argc, char *argv[])
{
	int opt;
	int err;
	int i;
	char err_buf[ERR_BUFFER_SIZE];
	FILE *fp;
	regex_t pat;

	/* optional analysis */
	while ((opt = getopt_long(argc, argv, "iv", NULL, NULL)) != -1)
	{
		switch (opt)
		{
			case 'i':
				option_flag |= IGNORE_UPPERLOWER;
				break;
			case 'v':
				option_flag |= WITHOUT_PATTERN;
				break;
			case '?':
				/* fall through */
			default:
				fprintf(stderr, "Usage: %s [option...] [file...]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	/* compile regex */
	err = regcomp(
		&pat,
		(option_flag & IGNORE_UPPERLOWER) ? lower_to_upper(argv[optind]) : argv[optind],
		REG_EXTENDED | REG_NOSUB | REG_NEWLINE
	);
	if (err != 0)
	{
		regerror(err, &pat, err_buf, sizeof(err_buf));
		fprintf(stderr, "%s\n", err_buf);
		exit(EXIT_FAILURE);
	}

	if (argc == 2)
	{
		do_grep(&pat, stdin);
	}
	else
	{
		for (i = optind + 1; i < argc; i++)
		{
			fp = fopen(argv[i], "r");
			if (!fp)
			{
				perror(argv[i]);
				regfree(&pat);
				exit(EXIT_FAILURE);
			}

			do_grep(&pat, fp);

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

static void do_grep(regex_t *pat, FILE *fp)
{
	char buf[BUFFER_SIZE];
	int is_unmatch;

	while (fgets(buf, sizeof(buf), fp))
	{
		is_unmatch = regexec(pat, ((option_flag & IGNORE_UPPERLOWER) ? lower_to_upper(buf) : buf), 0, NULL, 0);
		if (is_unmatch)
		{
			/* unmatch */
			if (option_flag & WITHOUT_PATTERN) printf("%s", buf);
		}
		else
		{
			/* match */
			if (!(option_flag & WITHOUT_PATTERN)) printf("%s", buf);
		}

	}
}

static char *lower_to_upper(char *str)
{
	static char ret_str[BUFFER_SIZE];
	int i;

	for (i = 0; i < sizeof(ret_str); i++)
	{
		ret_str[i] = toupper(str[i]);
		if (ret_str[i] == '\0') break;
	}

	return ret_str;
}


