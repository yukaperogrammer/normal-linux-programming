#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include <stdio.h>
#define MAX_REQUEST_BODY_LENGTH (1024 * 1024)

struct HTTPHeaderField {
	char *name;
	char *value;
	struct HTTPHeaderField *next;
};

struct HTTPRequest {
	int protocol_minor_version;
	char *method;
	char *path;
	struct HTTPHeaderField *header;
	char *body;
	long length;
};

struct HTTPRequest *read_request(FILE *);
void read_request_line(struct HTTPRequest *, FILE *);
struct HTTPHeaderField *read_header_field(FILE *);
long content_length(struct HTTPRequest *);
char *lookup_header_field_value(struct HTTPRequest *, char *);
void respond_to(struct HTTPRequest *, FILE *, char *);
void free_request(struct HTTPRequest *);

#endif

