#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	pid_t pid;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <command> <arg>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	pid = fork();
	
	if (pid < 0)
	{
		fprintf(stderr, "fork(2) failed\n");
		exit(EXIT_FAILURE);
	}

	if (pid == 0)
	{
		/* child process */
		execl(argv[1], argv[1], argv[2], NULL);

		/* execl return, failed */
		perror(argv[1]);
		exit(99);
	}
	else
	{
		/* parent process */
		int status;

		waitpid(pid, &status, 0);
		printf("child (PID=%d) finished; ", pid);

		if (WIFEXITED(status))
		{
			printf("exit, status=%d\n", WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			printf("signal, sig=%d\n", WTERMSIG((status)));
		}
		else
		{
			printf("abnormal exit\n");
		}

	}

	return 0;
}

