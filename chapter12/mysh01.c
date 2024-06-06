#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <wait.h>

#define PROMPT "$ "
#define BUF_SIZE 1024
#define CAPA 16

static char *program_name;

struct cmd {
	char **argv;
	long argc;
	long capa;
};

static void *xmalloc(size_t);
static void *xrealloc(void *, size_t);
static void free_cmd(struct cmd *);
static struct cmd *read_cmd(void);
static struct cmd *parse_cmd(char *);
static void invoke_cmd(struct cmd *);

int main(int argc, char *argv[])
{
	if (argc != 1) {
		fprintf(stderr, "Usage: %s(boot shell)\n", argv[0]);
		exit(1);
	}

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

static void *xmalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if(!p) {
		fprintf(stderr, "memory allocation error\n");
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
		fprintf(stderr, "memory allocation error\n");
		exit(1);
	}

	return p;
}

static struct cmd *read_cmd(void)
{
	static char buf[BUF_SIZE];

	if (fgets(buf, BUF_SIZE, stdin) == NULL) {
		exit(0);
	}

	return parse_cmd(buf);
}

static struct cmd *parse_cmd(char *cmdline)
{
	char *p = cmdline;
	struct cmd *cmd;

	cmd = (struct cmd *)xmalloc(sizeof(struct cmd));
	cmd->argv = (char **)xmalloc(sizeof(char *) * CAPA);
	cmd->argc = 0;
	cmd->capa = CAPA;

	while (*p) {
		while (*p && isspace(*p)) {
			*p++ = '\0';
		}

		if (*p) {
			if (cmd->argc + 1 >= cmd->capa) {
				cmd->capa += CAPA;
				cmd->argv = (char **)xrealloc(cmd->argv, cmd->capa);
			}
		
			cmd->argv[cmd->argc] = p;
			cmd->argc++;
		}

		while (*p && !isspace(*p)) {
			p++;
		}

	}
	cmd->argv[cmd->argc] = NULL;

	return cmd;
}

static void invoke_cmd(struct cmd *cmd)
{
	pid_t pid;

	pid = fork();

	if (pid < 0) {
		perror(program_name);
		exit(99);
	}

	if (pid > 0) {
		waitpid(pid, NULL, 0);
	}
	else {
		execvp(cmd->argv[0], cmd->argv);

		fprintf(stderr, "%s: command not found: %s\n", program_name, cmd->argv[0]);
		exit(1);
	}
}

static void free_cmd(struct cmd *cmd)
{
	free(cmd->argv);
	free(cmd);
}

