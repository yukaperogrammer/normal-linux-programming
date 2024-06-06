#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#define LINE_BUF_SIZE 4096
#define BLOCK_BUF_SIZE 1024
#define TIME_BUF_SIZE 64
#define MAX_REQUEST_BODY_LENGTH (1024 * 1024)
#define SERVER_NAME "LittleHTTP"
#define SERVER_VERSION "1.0"
#define HTTP_MINOR_VERSION 0

/* structures */
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

struct FileInfo {
	char *path;
	long size;
	int ok;
};

/* define types */
typedef void                   (*sighandler_t)(int);
typedef struct HTTPHeaderField http_headerfield;
typedef struct HTTPRequest     http_request;
typedef struct FileInfo        fileinfo;

/* function prototypes */
static void log_exit(char *fmt, ...);
static void *xmalloc(size_t);
static void install_signal_handlers(void);
static void trap_signal(int, sighandler_t);
static void signal_exit(int);
static void service(FILE *, FILE *, char *);
static void free_request(http_request *);
static void free_fileinfo(fileinfo *);
static http_request *read_request(FILE *);
static void read_request_line(http_request *, FILE *);
static void upcase(char *);
static http_headerfield *read_header_field(FILE *);
static long content_length(http_request *);
static char *lookup_header_field_value(http_request *, char *);
static fileinfo *get_fileinfo(char *, char *);
static char *build_fspath(char *, char *);
static void respond_to(http_request *, FILE *, char *); 
static void do_file_response(http_request *, FILE *, char *);
static void not_found(http_request *, FILE *); 
static void output_common_header_field(http_request *, FILE *, char *);
static char *guess_content_type(fileinfo *);
static void method_not_allowed(http_request *, FILE *);
static void not_implemented(http_request *, FILE *);

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
	http_request *req;

	req = read_request(in);
	respond_to(req, out, docroot);
	free_request(req);
}

static void log_exit(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);
	exit(1);
}

static void *xmalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (!p) {
		log_exit("failed to allocate memory");
	}

	return p;
}

static void install_signal_handlers(void)
{
	trap_signal(SIGPIPE, signal_exit);
}

static void trap_signal(int sig, sighandler_t handler)
{
	struct sigaction act;

	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(sig, &act, NULL) < 0) {
		log_exit("sigaction() failed", strerror(errno));
	}
}

static void signal_exit(int sig)
{
	log_exit("exit by signal %d", sig);
}

static void free_request(http_request *req)
{
	http_headerfield *h, *head;

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

static void free_fileinfo(fileinfo *info)
{
	free(info->path);
	free(info);
}

static http_request *read_request(FILE *in)
{
	http_request *req;
	http_headerfield *h;

	req = xmalloc(sizeof(http_request));
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

static void read_request_line(http_request *req, FILE *in)
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

static void upcase(char *str)
{
	char *p;

	for (p = str; *p; p++) {
		*p = toupper(*p);
	}
}

static http_headerfield *read_header_field(FILE *in)
{
	http_headerfield *h;
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
		log_exit("parse error on request field: %s", buf);
	}
	*p++ = '\0';
	h = xmalloc(sizeof(http_headerfield));
	h->name = xmalloc(p - buf);
	strcpy(h->name, buf);

	p += strspn(p, " \t");
	h->value = xmalloc(strlen(p) + 1);
	strcpy(h->value, p);

	return h;
}

static long content_length(http_request *req)
{
	char *val;
	long len;

	val = lookup_header_field_value(req, "Content-Length");
	if (!val) {
		return 0;
	}

	len = atol(val);
	if (len < 0) {
		log_exit("negative Content_Length value");
	}

	return len;
}

static char *lookup_header_field_value(http_request *req, char *name)
{
	http_headerfield *h;

	for (h = req->header; h; h = h->next) {
		if (strcasecmp(h->name, name) == 0) {
			return h->value;
		}
	}

	return NULL;
}

static fileinfo *get_fileinfo(char *docroot, char *urlpath)
{
	fileinfo *info;
	struct stat st;

	info = xmalloc(sizeof(fileinfo));
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

static char *build_fspath(char *docroot, char *urlpath)
{
	char *path;

	path = xmalloc(strlen(docroot) + 1 + strlen(urlpath) + 1);
	sprintf(path, "%s/%s", docroot, urlpath);

	return path;
}

static void respond_to(http_request *req, FILE *out, char *docroot)
{
	if (strcmp(req->method, "GET") == 0) {
		do_file_response(req, out, docroot);
	} else if (strcmp(req->method, "HEAD") == 0) {
		do_file_response(req, out, docroot);
	} else if (strcmp(req->method, "POST")) {
		method_not_allowed(req, out);
	} else {
		not_implemented(req, out);
	}
}

static void do_file_response(http_request *req, FILE *out, char *docroot)
{
	fileinfo *info;

	info = get_fileinfo(docroot, req->path);
	if (!info->ok) {
		free_fileinfo(info);
		not_found(req, out);
		return;
	}

	output_common_header_field(req, out, "200 OK");
	fprintf(out, "Content-Lnegth: %ld\r\n", info->size);
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

static void not_found(http_request *req, FILE *out)
{
	output_common_header_field(req, out, "404 Not Found");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	if (strcmp(req->method, "HEAD") != 0) {
		fprintf(out, "<html>\r\n");
		fprintf(out, "<header><title>Not Found</fifle></header>\r\n");
		fprintf(out, "<body><p>File not found</p></body>\r\n");
		fprintf(out, "</html>\r\n");
	}
	fflush(out);
}

static void output_common_header_field(http_request *req, FILE *out, char *status)
{
	time_t t;
	struct tm *tm;
	char buf[TIME_BUF_SIZE];

	t = time(NULL);
	tm = gmtime(&t);
	if (!tm) {
		log_exit("gmtime() failed: %s", strerror(errno));
	}
	strftime(buf, TIME_BUF_SIZE, "%a %d %b %Y %H:%M:%S GMT", tm);
	fprintf(out, "HTTP/1.%d %s\r\n", HTTP_MINOR_VERSION, status);
	fprintf(out, "Date: %s\r\n", buf);
	fprintf(out, "Server: %s/%s\r\n", SERVER_NAME, SERVER_VERSION);
	fprintf(out, "Connection: close\r\n");
}

static char *guess_content_type(fileinfo *info)
{
	return "text/plain";
}

static void method_not_allowed(http_request *req, FILE *out)
{
	output_common_header_field(req, out, "405 Method Not Allowed");
	fprintf(out, "Content-Type: text/html\r\n");
	fprintf(out, "\r\n");
	fprintf(out, "<html>\r\n");
	fprintf(out, "<header>\r\n");
	fprintf(out, "<title>405 Method Not Allowed</title>\r\n");
	fprintf(out, "</header>>\r\n");
	fprintf(out, "<body>\r\n");
	fprintf(out, "<p>The request method %s is not allowed</p>\r\n", req->method);
	fprintf(out, "</body>\r\n");
	fprintf(out, "</html>\r\n");
	fflush(out);
}

static void not_implemented(http_request *req, FILE *out)
{
	output_common_header_field(req, out, "501 Not Implemented");
	fprintf(out, "Content-type: text/html\r\n");
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

