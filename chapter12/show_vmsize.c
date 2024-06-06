#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BUFSIZE 256

void use_system(const char *);

int main(int argc, char *argv[])
{
	pid_t pid;

	if (argc != 1)
	{
		fprintf(stderr, "Usage: no argument\n");
		exit(1);
	}

	use_system("running parent process only...");
	
	pid = fork();

	if (pid < 0)
	{
		fprintf(stderr, "make child process failed\n");
		exit(1);
	}

	if (pid == 0)
	{
		/* child process*/
		int c;

		printf("Press 'q' to exit\n");
		while ((c = fgetc(stdin)))
		{
			if (c == 'q')
			{
				exit(0);
			}
		}
	}
	else
	{
		/* parent process */
		int status;

		use_system("running parent and child process...");

		waitpid(pid, &status, 0);

		use_system("exit child process..."); 

		printf("child (PID=%d) finished: ", pid);
		if (WIFEXITED(status))
		{
			printf("exit, status=%d\n", WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			printf("signale, status=%d\n", WTERMSIG(status));
		}
		else
		{
			printf("abnormal exit\n");
		}
	}

	return 0;
}

void use_system(const char *string)
{
	char command[BUFSIZE];

	printf("%s\n", string);

	sprintf(command, "grep VmSize /proc/%d/status", getpid());
	system(command);
}

