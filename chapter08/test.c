#include <stdio.h>

static void test_func(void);

int main(int argc, char *argv[]) {
	printf("%s\n", __func__);
	test_func();

	return 0;
}

static void test_func(void) {
	printf("%s\n", __func__);
}

