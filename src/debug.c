#include <breakpoint.h>
#include <gdd.h>
#include <gdd_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

int debug_child() {
	while (1) {
		int status;
		waitpid(child_pid, &status, 0);
		if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
			printf("Stopped at: DUNNO? \n\n");
			handle_break();
		} else if (WIFEXITED(status)) {
			printf("Child exited.\n");
			printf("Terminating...\n");
			return 0;
		}
	}
}

void handle_break() {
	breakpoint_data();

	printf("> ");

	char instruction[MAX_INSTRUCTION_LEN];
	fgets(instruction, MAX_INSTRUCTION_LEN, stdin);
	int len = strlen(instruction);

	if (len > 0 && instruction[len - 1] == '\n') {
		instruction[strlen(instruction) - 1] = '\0';
		len--;
	}

	if (strcmp(instruction, "c") == 0 || strcmp(instruction, "r") == 0) {
		if (instruction[0] == 'c') {
			resume_from_breakpoint();
		} else {
			breakpoint_continue();
		}
		return;

	} else if (instruction[0] == 'b' && instruction[1] == ' ') {

		int line = my_atoi(&instruction[2], len - 2);
		if (line == -1) {
			printf("Invalid line number: %s \n", &instruction[2]);
			return handle_break();
		}
		set_breakpoint();
		return handle_break();

	} else if (strcmp(instruction, "s") == 0) {
		one_step();
	} else {
		printf("Unknown instruction: %s \n", instruction);
		return handle_break();
	}
}
