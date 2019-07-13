#include <gdd.h>
#include <stdio.h>

word my_atoi(char *str, int len) {
	word code = 0;

	for (int i = 0; i < len; i++) {
		if (str[i] < 48 || str[i] > 57) {
			return -1;
		}
		code += (int)str[i] - 48;
		if (i < len - 1) {
			code *= 10;
		}
	}

	return code;
}

// debugging purposes
void print_word(char *label, word to_print) {
	printf("%s: 0x%08llx \n", label, to_print);
}
