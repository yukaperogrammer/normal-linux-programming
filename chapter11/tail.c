#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_LINES 10
#define LINE_BUFSIZE  256

static void do_tail(FILE *, long);
static void print_tail(char **, int, long);

int main(int argc, char *argv[])
{
	int opt;
	int i;
	long nlines = DEFAULT_LINES;
	FILE *fp;

	while ((opt = getopt(argc, argv, "n:")) != -1)
	{
		switch (opt)
		{
			case 'n':
				nlines = atol(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [n <lines>] [<flie> [...[<file>]]]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (optind == argc)
	{
		do_tail(stdin, nlines);
	}
	else
	{
		for (i = optind; i < argc; i++)
		{
			fp = fopen(argv[i], "r");
			if (!fp)
			{
				fprintf(stderr, "%s: open file failure\n", argv[i]);
				continue;
			}

			do_tail(fp, nlines);

			if (fclose(fp) < 0)
			{
				fprintf(stderr, "%s: close file failure\n", argv[i]);
				continue;
			}
		}
	}

	return 0;
}

static void do_tail(FILE *fp, long n)
{
	char **ptr;
	char buf[LINE_BUFSIZE] = "";
	int i;
	int j;
	int length;
	int ring_buf_ind = 0;

	ptr = (char **)malloc(sizeof(char *) * n);
	if (!ptr)
	{
		fprintf(stderr, "memory allocation error!\n");
		return;
	}

	for (i = 0; i < n; i++)
	{
		ptr[i] = (char *)malloc(sizeof(buf));
		if (!ptr[i])
		{
			/* all memory free */
			fprintf(stderr, "memory allocation error!\n");
			for (j = 0; j < i; j++)
			{
				free(ptr[j]);
			}
			free(ptr);
			return;
		}
	}

	/* lines copy into ring buffer */
	while (fgets(buf, sizeof(buf), fp))
	{
		length = strlen(buf);
		for (i = 0; i < length + 1; i++)
		{
			ptr[ring_buf_ind][i] = buf[i];
		}

		ring_buf_ind = (ring_buf_ind + 1) % n;
	}

	/* print n lines of tail */
	print_tail(ptr, ring_buf_ind, n);

	return;
}

static void print_tail(char **ptr, int begin, long limit)
{
	int i;
	int count;

	for (i = begin, count = limit; count > 0; i++, count--)
	{
		if (i >= limit) i = 0;
		printf("%s", ptr[i]);
	}

	return;
}

