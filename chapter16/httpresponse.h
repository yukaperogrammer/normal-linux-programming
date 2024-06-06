#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include <stdio.h>
#include "httprequest.h"
#define HTTP_MINOR_VERSION 0
#define SERVER_NAME "LittleHTTP"
#define SERVER_VERSION "1.0"

struct FileInfo {
	char *path;
	long size;
	int ok;
};

void respond_to(struct HTTPRequest *, FILE *, char *);
struct FileInfo *get_fileinfo(char *, char *);
void do_file_response(struct HTTPRequest *, FILE *, char *);
void output_common_header_fields(struct HTTPRequest *, FILE *, char *);
char *build_fspath(char *, char *);
void free_fileinfo(struct FileInfo *);
void method_not_allowed(struct HTTPRequest *, FILE *);
void not_implemented(struct HTTPRequest *, FILE *);
void not_found(struct HTTPRequest *, FILE *);
char *guess_content_type(struct FileInfo *);

#endif

