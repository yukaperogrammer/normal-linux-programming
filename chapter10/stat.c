#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

static char *file_type(mode_t);

int main(int argc, char *argv[])
{
	struct stat st;
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s filepath\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (lstat(argv[1], &st) < 0)
	{
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	printf("type\t%o (%s)\n", (st.st_mode & S_IFMT), file_type(st.st_mode));
	printf("mode\t%o\n", st.st_mode & ~S_IFMT);
	printf("dev\t%llu\n", (unsigned long long)st.st_dev);
	printf("ino\t%lu\n", (unsigned long)st.st_ino);
	printf("rdev\t%llu\n", (unsigned long long)st.st_rdev);
	printf("nlink\t%lu\n", (unsigned long)st.st_nlink);
	printf("uid\t%d\n", st.st_uid);
	printf("gid\t%d\n", st.st_gid);
	printf("size\t%ld\n", st.st_size);
	printf("blksize\t%lu\n", (unsigned long)st.st_blksize);
	printf("blocks\t%lu\n", (unsigned long)st.st_blocks);
	printf("atime\t%s", ctime(&st.st_atime));
	printf("mtime\t%s", ctime(&st.st_mtime));
	printf("ctime\t%s", ctime(&st.st_ctime));

	return 0;
}

static char *file_type(mode_t mode)
{
	if (S_ISREG(mode))   return "file";
	if (S_ISDIR(mode))   return "directory";
	if (S_ISCHR(mode))   return "chardev";
	if (S_ISBLK(mode))   return "blockdev";
	if (S_ISFIFO(mode))  return "fifo";
	if (S_ISLNK(mode))   return "symlink";
	if (S_ISSOCK(mode)) return "socket";

	return "unknown";
}

