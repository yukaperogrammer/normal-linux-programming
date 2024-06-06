#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include "util.h"

extern int debug_mode;

/* log */
void log_exit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (debug_mode) {
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
	} else {
		vsyslog(LOG_ERR, fmt, ap);
	}

	va_end(ap);

	exit(1);
}

/* memory allocation */
void *xmalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (!p) {
		log_exit("failed to allocate memory");
	}

	return p;
}

/* signal handler */
void install_signal_handlers(void)
{
	trap_signal(SIGPIPE, signal_exit);
	detach_children();
}

void trap_signal(int sig, sighandler_t handler)
{
	struct sigaction act;

	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(sig, &act, NULL) < 0) {
		log_exit("sigaction() failed: %s", strerror(errno));
	}
}

void detach_children(void)
{
	struct sigaction act;

	act.sa_handler = noop_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
	if (sigaction(SIGCHLD, &act, NULL) < 0) {
		log_exit("sigaction() failed: %s", strerror(errno));
	}
}

void noop_handler(int sig)
{
	;
}

void signal_exit(int sig)
{
	log_exit("exit by signal %d", sig);
}

/* etc... */
void upcase(char *str)
{
	char *p = str;

	while (*p != '\0') {
		*p = toupper(*p);
		p++;
	}
}

