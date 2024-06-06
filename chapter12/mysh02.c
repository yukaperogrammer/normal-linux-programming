#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <wait.h>

#define PROMPT "$ "
#define BUFSIZE 1024
#define CAPA 16
#define COMMANDS 2 

struct cmd {
	long argc;
	long capa;
	char **argv;
};

static char *program_name;

static void invoke_cmd(struct cmd **);
static void parent_process(pid_t, int *);
static void child_process(struct cmd **, int *);
static void *xmalloc(size_t);
static void *xrealloc(void *, size_t);
static void free_cmd(struct cmd **);
static struct cmd **read_cmd(void);
static struct cmd **parse_cmd(char *);

int main(int argc, char *argv[])
{
	if (argc != 1) {
		fprintf(stderr, "Usage: %s (no argument)\n", argv[0]);
		exit(1);
	}

	program_name = argv[0];

	for (;;) {
		struct cmd **cmd;

		fprintf(stdout, PROMPT);
		fflush(stdout);

		cmd = read_cmd();

		if (cmd[0]->argc > 0) {
			invoke_cmd(cmd);
		}

		free_cmd(cmd);
	}

	return 0;
}

static void invoke_cmd(struct cmd **cmd)
{
	pid_t pid;
	int fds[2];
	
	pipe(fds);
	pid = fork();

	if (pid < 0) {
		perror("fork");
		exit(1);
	}

	if (pid > 0) {
		parent_process(pid, fds);
	}
	else {
		child_process(cmd, fds);
		fprintf(stderr, "%s: command not found %s\n",program_name, cmd->argv[0]);
		exit(1);
	}
}

static void parent_process(pid_t pid, int *fds)
{
	close(fds[0]);
	waitpid(pid, NULL, 0);
	exit(0);
}

static void child_process(struct cmd **cmd, int *fds)
{
	close(fds[1]);

	if (!cmd) {
		close(stdout);
		dup2(fds[0], stdout);
		close(fds[0]);
		exit(0);
	}

	invoke_process(cmd + 1);
	execvp(*cmd->argv[0], *cmd->argv);
}

static void *xmalloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr) {
		perror("malloc");
		exit(1);
	}

	return ptr;
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

static struct cmd **read_cmd(void)
{
	static char buf[BUFSIZE];

	if (fgets(buf, BUFSIZE, stdin) == NULL) {
		/* Ctrl + C */
		exit(0);
	}

	return parse_cmd(buf);
}

static struct cmd **parse_cmd(char *cmdline)
{
	struct cmd **cmd;
	char *p = cmdline;
	int cmd_ind = 0;

	cmd = (struct cmd **)xmalloc(sizeof(struct cmd *) * COMMANDS);
	cmd[cmd_ind] = (struct cmd *)xmalloc(sizeof(struct cmd *));
	cmd[cmd_ind]->argc = 0;
	cmd[cmd_ind]->capa = CAPA;
	cmd[cmd_ind]->argv = (char **)xmalloc(sizeof(char *) * CAPA);
	cmd[cmd_ind + 1] = NULL;

	while (*p) {
		while (*p && isspace(*p)) {
			*p++ = '\0';
		}

		if (*p) {
			if (cmd[cmd_ind]->argc + 1 >= cmd[cmd_ind]->capa) {
				cmd[cmd_ind]->capa += CAPA;
				cmd[cmd_ind]->argv = (char **)xrealloc(cmd[cmd_ind]->argv, cmd[cmd_ind]->capa);
			}

			if (*p == '|') {
				cmd[cmd_ind]->argv[cmd[cmd_ind]->argc] = NULL;
				cmd_ind++;
				cmd = (struct cmd **)xrealloc(cmd, cmd_ind + 2);
				cmd[cmd_ind] = (struct cmd *)xmalloc(sizeof(struct cmd *));
				cmd[cmd_ind]->argc = 0;
				cmd[cmd_ind]->capa = CAPA;
				cmd[cmd_ind]->argv = (char **)xmalloc(sizeof(char *) * CAPA);
				cmd[cmd_ind + 1] = NULL;
			}
			else {
				cmd[cmd_ind]->argv[cmd[cmd_ind]->argc] = p;
				cmd[cmd_ind]->argc++;
			}
		}

		while (*p && !isspace(*p)) {
			p++;
		}
	}
	cmd[cmd_ind]->argv[cmd[cmd_ind]->argc] = NULL;

	return cmd;
}

static void free_cmd(struct cmd **cmd)
{
	struct cmd **ptr = cmd;
	int i;

	for (i = 0; ptr + i; i++) {
		free(ptr[i]->argv);
	}

	free(cmd);
}

