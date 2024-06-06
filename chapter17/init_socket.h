#ifndef INIT_SOCKET_H
#define INIT_SOCKET_H

#define MAX_BACKLOG 5

int listen_socket(char *);
void setup_environment(char *, char *, char *);
void server_main(int, char *);
void service(FILE *in, FILE *out, char *);
void become_daemon(void);

#endif

