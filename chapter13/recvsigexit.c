#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef void (*sighandler_t)(int);

sighandler_t trap_signal(int, sighandler_t);
void handler_sigint(int);

int main(int argc, char *argv[])
{
	if (argc != 1) {
		fprintf(stderr, "Usage: %s(no argument)\n", argv[0]);
		exit(1);
	}

	trap_signal(SIGINT, handler_sigint);
	printf("signal waiting...\n");
	pause();

	return 0;
}

sighandler_t trap_signal(int sig, sighandler_t handler)
{
	struct sigaction act, old;

	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(sig, &act, &old) < 0) {
		return NULL;
	}

	return old.sa_handler;
}

void handler_sigint(int sig)
{
	printf("receive signal: SIGINT\n");
}

