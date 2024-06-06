#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "httprequest.h"
#include "util.h"

#define LINE_BUF_SIZE 256

struct HTTPRequest *read_request(FILE *in)
{
	struct HTTPRequest *req;
	struct HTTPHeaderField *h;

	req = xmalloc(sizeof(struct HTTPRequest));
	read_request_line(req, in);
	req->header = NULL;

	while ((h = read_header_field(in))) {
		h->next = req->header;
		req->header = h;
	}

	req->length = content_length(req);
	if (req->length != 0) {
		if (req->length > MAX_REQUEST_BODY_LENGTH) {
			log_exit("request body too long");
		}

		req->body = xmalloc(req->length);
		if (fread(req->body, req->length, 1, in) < 1) {
			log_exit("failed to read request body");
		}
	} else {
		req->body = NULL;
	}

	return req;
}

void read_request_line(struct HTTPRequest *req, FILE *in)
{
	char buf[LINE_BUF_SIZE];
	char *path, *p;

	if (!fgets(buf, LINE_BUF_SIZE, in)) {
		log_exit("no request line");
	}

	p = strchr(buf, ' ');
	if (!p) {
		log_exit("parse error on request line (1): %s", buf);
	}
	*p++ = '\0';
	req->method = xmalloc(p - buf);
	strcpy(req->method, buf);
	upcase(req->method);

	path = p;
	p = strchr(path, ' ');
	if (!p) {
		log_exit("parse error on request line (2): %s", buf);
	}
	*p++ = '\0';
	req->path = xmalloc(p - path);
	strcpy(req->path, path);

	if (strncasecmp(p, "HTTP/1.", strlen("HTTP/1.")) != 0) {
		log_exit("parse error on request line (3): %s", buf);
	}
	p += strlen("HTTP/1.");
	req->protocol_minor_version = atoi(p);
}

struct HTTPHeaderField *read_header_field(FILE *in)
{
	struct HTTPHeaderField *h;
	char buf[LINE_BUF_SIZE];
	char *p;

	if (!fgets(buf, LINE_BUF_SIZE, in)) {
		log_exit("failed to read request header field: %s", strerror(errno));
	}
	if ((buf[0] == '\n') || (strcmp(buf, "\r\n") == 0)) {
		return NULL;
	}

	p = strchr(buf, ':');
	if (!p) {
		log_exit("parse error on request header field: %s", buf);
	}
	*p++ = '\0';
	h = xmalloc(sizeof(struct HTTPHeaderField));
	h->name = xmalloc(p - buf);
	strcpy(h->name, buf);

	p += strspn(p, " \t");
	h->value = xmalloc(strlen(p) + 1);
	strcpy(h->value, p);

	return h;
}

long content_length(struct HTTPRequest *req)
{
	char *val;
	long len;

	val = lookup_header_field_value(req, "Content-Length");
	if (!val) {
		return 0;
	}

	len = atol(val);
	if (len < 0) {
		log_exit("negative Content-Length value");
	}

	return len;
}

char *lookup_header_field_value(struct HTTPRequest *req, char *name)
{
	struct HTTPHeaderField *h;

	for (h = req->header; h; h = h->next) {
		if (strcasecmp(h->name, name) == 0) {
			return h->value;
		}
	}

	return NULL;
}

void free_request(struct HTTPRequest *req)
{
	struct HTTPHeaderField *h, *head;

	head = req->header;
	while (head) {
		h = head;
		head = head->next;
		free(h->name);
		free(h->value);
		free(h);
	}

	free(req->method);
	free(req->path);
	free(req->body);
	free(req);
}

