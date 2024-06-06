#ifndef UTIL_H
#define UTIL_H

typedef void (*sighandler_t)(int);

/* log */
void log_exit(char *, ...);

/* memory allocation */
void *xmalloc(size_t);

/* signal handler */
void install_signal_handlers(void);
void trap_signal(int, sighandler_t);
void signal_exit(int);

/* etc... */
void upcase(char *);

#endif

