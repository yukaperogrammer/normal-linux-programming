#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
	struct passwd *pw;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <username>\n", argv[0]);
		exit(1);
	}

	if (!(pw = getpwnam(argv[1]))) {
		fprintf(stderr, "%s is not found\n", argv[1]);
		exit(1);
	}

	printf("username: %s\n", pw->pw_name);
	printf("password: %s\n", pw->pw_passwd);
	printf("userid:   %d\n", pw->pw_uid);
	printf("groupid:  %d\n", pw->pw_gid);
	printf("homedir:  %s\n", pw->pw_dir);
	printf("shell:    %s\n", pw->pw_shell);

	return 0;
}

