#include <stdio.h>
#include "src/test.h"
#include "src/subdir2/test2.h"



int main() {
	printf("Hello Make World\n");
	print_test();
	print_test_2();
	return 0;
}
