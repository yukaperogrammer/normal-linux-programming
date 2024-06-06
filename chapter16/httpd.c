#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "httprequest.h"
#include "httpresponse.h"

static void service(FILE *, FILE *, char *);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <docroot>\n", argv[0]);
		exit(1);
	}

	install_signal_handlers();
	service(stdin, stdout, argv[1]);

	return 0;
}

static void service(FILE *in, FILE *out, char *docroot)
{
	struct HTTPRequest *req;

	req = read_request(in);
	respond_to(req, out, docroot);
	free_request(req);
}

