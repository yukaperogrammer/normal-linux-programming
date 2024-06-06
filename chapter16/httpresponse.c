#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "httpresponse.h"
#include "httprequest.h"
#include "util.h"
#define BLOCK_BUF_SIZE 1024
#define TIME_BUF_SIZE 64 

void respond_to(struct HTTPRequest *req, FILE *out, char *docroot)
{
	if (strcmp(req->method, "GET") == 0) {
		do_file_response(req, out, docroot);
	} else if (strcmp(req->method, "HEAD") == 0) {
		do_file_response(req, out, docroot);
	} else if (strcmp(req->method, "POST") == 0) {
		method_not_allowed(req, out);
	} else {
		not_implemented(req, out);
	}
}

void do_file_response(struct HTTPRequest *req, FILE *out, char *docroot)
{
	struct FileInfo *info;

	info = get_fileinfo(docroot, req->path);
	if (!info->ok) {
		free_fileinfo(info);
		not_found(req, out);
		return;
	}

	output_common_header_fields(req, out, "200 OK");
	fprintf(out, "Content-Length: %ld\r\n", info->size);
	fprintf(out, "Content-Type: %s\r\n", guess_content_type(info));
	fprintf(out, "\r\n");

	if (strcmp(req->method, "HEAD") != 0) {
		int fd;
		char buf[BLOCK_BUF_SIZE];
		ssize_t n;

		fd = open(info->path, O_RDONLY);
		if (fd < 0) {
			log_exit("failed to open %s: %s", info->path, strerror(errno));
		}

		for (;;) {
			n = read(fd, buf, BLOCK_BUF_SIZE);
			if (n < 0) {
				log_exit("failed to read %s: %s", info->path, strerror(errno));
			}

			if (n == 0) {
				break;
			}

			if (fwrite(buf, 1, n, out) < n) {
				log_exit("failed to write to socket: %s", strerror(errno));
			}
		}
		close(fd);
	}

	fflush(out);
	free_fileinfo(info);
}

struct FileInfo *get_fileinfo(char *docroot, char *urlpath)
{
	struct FileInfo *info;
	struct stat st;

	info = xmalloc(sizeof(struct FileInfo));
	info->path = build_fspath(docroot, urlpath);
	info->ok = 0;
	if (lstat(info->path, &st) < 0) {
		return info;
	}
	if (!S_ISREG(st.st_mode)) {
		return info;
	}

	info->ok = 1;
	info->size = st.st_size;

	return info;
}

void output_common_header_fields(struct HTTPRequest *req, FILE *out, char *status)
{
	time_t t;
	struct tm *tm;
	char buf[TIME_BUF_SIZE];

	t = time(NULL);
	tm = gmtime(&t);
	if (!tm) {
		log_exit("gmtime() failed: %s", strerror(errno));
	}

	strftime(buf, TIME_BUF_SIZE, "%a, %d %b %Y %H:%M:%S GMT", tm);
	fprintf(out, "HTTP/1.%d %s\r\n", HTTP_MINOR_VERSION, status);
	fprintf(out, "Date: %s\r\n", buf);
	fprintf(out, "Server: %s/%s\r\n", SERVER_NAME, SERVER_VERSION);
	fprintf(out, "Connection: close\r\n");
}

char *build_fspath(char *docroot, char *urlpath)
{
	char *path;

	path = xmalloc(strlen(docroot) + 1 + strlen(urlpath) + 1);
	sprintf(path, "%s/%s", docroot, urlpath);

	return path;
}

void free_fileinfo(struct FileInfo *info)
{
	free(info->path);
	free(info);
}

void method_not_allowed(struct HTTPRequest *req, FILE *out)
{
	output_common_header_fields(req, out, "405 Method Not Allowed");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	fprintf(out, "<html>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<title>405 Method Not Allowed</title>\r\n");
	fprintf(out, "</header>\r\n");
	fprintf(out, "<body>\r\n");
	fprintf(out, "<p>The request method %s is not allowed</p>\r\n", req->method);
	fprintf(out, "</body>\r\n");
	fprintf(out, "</html>\r\n");
	fflush(out);
}

void not_implemented(struct HTTPRequest *req, FILE *out)
{
	output_common_header_fields(req, out, "501 Not Implemented");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	fprintf(out, "<html>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<title>501 Not Implemented</title>\r\n");
	fprintf(out, "</header>\r\n");
	fprintf(out, "<body>\r\n");
	fprintf(out, "<p>The request method %s is not implemented</p>\r\n", req->method);
	fprintf(out, "</body>\r\n");
	fprintf(out, "</html>\r\n");
	fflush(out);
}

void not_found(struct HTTPRequest *req, FILE *out)
{
	output_common_header_fields(req, out, "404 Not Found");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	if (strcmp(req->method, "HEAD") != 0) {
		fprintf(out, "<html>\r\n");
		fprintf(out, "<header><title>Not Found</title></header>\r\n");
		fprintf(out, "<body><p>File not found</p></body>\r\n");
		fprintf(out, "</html>\r\n");
	}
	fflush(out);
}

char *guess_content_type(struct FileInfo *info)
{
	return "text/plain";
}

