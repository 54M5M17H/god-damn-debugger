#include "test.h"
#include <stdio.h>

int main() {
	int mult = 2;
	int e = adder(3, 4);
	int res = e * mult;
	printf("answer is %i\n", res);

	return 0;
}
// TODO: test a loop