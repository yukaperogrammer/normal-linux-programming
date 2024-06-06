#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

#define BUFSIZE 2048
#define INITCAPA 16
#define PROMPT "$ "

struct cmd {
	char **argv;
	long argc;
	long capa;
};

static char *program_name;

static void invoke_cmd(struct cmd *);
static struct cmd *read_cmd(void);
static struct cmd *parse_cmd(char *);
static void free_cmd(struct cmd *);
static void *xmalloc(size_t);
static void *xrealloc(void *, size_t);

int main(int argc, char *argv[])
{
	program_name = argv[0];

	for (;;) {
		struct cmd *cmd;

		fprintf(stdout, PROMPT);
		fflush(stdout);

		cmd = read_cmd();
		if (cmd->argc > 0) {
			invoke_cmd(cmd);
		}
		free_cmd(cmd);
	}

	return 0;
}

static void invoke_cmd(struct cmd *cmd)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}

	if (pid > 0) {
		/* parent process */
		waitpid(pid, NULL, 0);
	}
	else {
		/* child process */
		execvp(cmd->argv[0], cmd->argv);

		/* error */
		fprintf(stderr, "%s: command not found: %s\n", program_name, cmd->argv[0]);
		exit(1);
	}
}

static struct cmd *read_cmd(void)
{
	static char buf[BUFSIZE];

	if (fgets(buf, BUFSIZE, stdin) == NULL) {
		exit(0);
	}

	return parse_cmd(buf);
}

static struct cmd *parse_cmd(char *cmdline)
{
	char *p = cmdline;
	struct cmd *cmd;

	cmd = (struct cmd *)xmalloc(sizeof(struct cmd));
	cmd->argc = 0;
	cmd->argv = (char **)xmalloc(sizeof(char *) * INITCAPA);
	cmd->capa = INITCAPA;

	while (*p) {
		while (*p && isspace((int)*p)) {
			*p++ = '\0';
		}

		if (*p) {
			if (cmd->capa <= cmd->argc + 1) { /* +1 for final NULL*/
				cmd->capa *= 2;
				cmd->argv = xrealloc(cmd->argv, cmd->capa);
			}
			cmd->argv[cmd->argc] = p;
			cmd->argc++;
		}

		while (*p && !isspace((int)*p)) {
			p++;
		}
	}
	cmd->argv[cmd->argc] = NULL;

	return cmd;
}

static void free_cmd(struct cmd *cmd)
{
	free(cmd->argv);
	free(cmd);
}

static void *xmalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (!p) {
		perror("malloc");
		exit(1);
	}

	return p;
}

static void *xrealloc(void *ptr, size_t size)
{
	void *p;

	if (!ptr) return xmalloc(size);
	p = realloc(ptr, size);
	if (!p) {
		perror("realloc");
		exit(1);
	}

	return p;
}

