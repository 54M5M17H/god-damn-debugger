int my_atoi(char *str, int len) {
	int code = 0;

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
