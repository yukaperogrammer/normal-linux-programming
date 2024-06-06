#include <stdio.h>

int main(int args, char *argv[]) {
	int i;

	for (int i = 0; i < args; i++) {
		printf("%d : %s\n", i, argv[i]);
	}

	return 0;
}

