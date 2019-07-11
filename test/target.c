#include "test.h"
#include <stdio.h>

int main() {
	int mult = 2;
	int e = adder(3, 4);
	// printf("e * mult is %i\n", e * mult);
	int res = e * mult;
	printf("answer is %i\n", res);
	return 0;
}
