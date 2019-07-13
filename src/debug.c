#include <gdd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

int MAX_INSTRUCTION_LEN = 20;

int get_command();
void parse_breakpoint_command(char *, int, struct file_and_line *);

bool STARTED = false;
char *command;

int start_debugger() {
	command = malloc(sizeof(char) * MAX_INSTRUCTION_LEN);
	int status;
	int res;

	while (1) {
		res = waitpid(child_pid, &status, 0);
		if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
			// printf("Stopped at: DUNNO? \n\n");
			handle_pause();
		} else if (WIFEXITED(status)) {
			printf("Child exited.\n");
			printf("Terminating...\n");
			return 0;
		} else if (WIFSIGNALED(status)) {
			printf("killed by signal %d\n", WTERMSIG(status));
		} else if (WIFSTOPPED(status)) {
			printf("stopped by signal %d\n", WSTOPSIG(status));
		} else if (WIFCONTINUED(status)) {
			printf("continued\n");
		} else {
			printf("Res: %i \n", res);
			printf("Unexpected Status: 0x%08x \n", status);
		}
	}
}

void handle_pause() {
	// breakpoint_data();

	printf("> ");
	int commandLength = get_command();

	if (strcmp(command, "r") == 0) {
		if (STARTED == true) {
			printf("Redundant command 'r'. Program has already started.\n");
			return handle_pause();
		}
		STARTED = true;
		ptrace_run();
		return;
	}

	if (strcmp(command, "c") == 0) {
		if (STARTED == false) {
			STARTED = true;
			ptrace_run();
			return;
		}

		// if (!ON_A_BREAKPOINT) {
		// TODO: COULD HAVE JUST STEPPED HERE, NO BPOINT
		// ptrace_run();
		// return;
		// }

		breakpoint_continue();
		return;
	}

	if (command[0] == 'b') {
		if (command[1] != ' ') {
			printf("Invalid breakpoint command \n");
			return handle_pause();
		}
		File_And_Line file_and_line = malloc(sizeof(File_And_Line));
		// struct file_and_line *file_and_line;
		parse_breakpoint_command(&command[2], commandLength - 2, file_and_line);
		if (file_and_line->line_number == -1) {
			return handle_pause();
		}

		set_breakpoint_at(file_and_line);
		return handle_pause();
	}

	if (command[0] == 'g') {
		ptrace_print_context();
		return handle_pause();
	}

	if (strcmp(command, "s") == 0) {
		// TODO: HANDLE THE CASE WHERE WE ARE ON OR CROSS
		// A BREAKPOINT AND NEED TO FIX THE INSTRUCTION
		ptrace_step_forward();
		// TODO: STEP INSTRUCTION VS STEP LINE

		return;
	}

	printf("Unknown command: %s \n", command);
	return handle_pause();
}

int get_command() {
	fgets(command, MAX_INSTRUCTION_LEN, stdin);
	int length = strlen(command);

	if (length > 0 && command[length - 1] == '\n') {
		command[strlen(command) - 1] = '\0';
		length--;
	}
	return length;
}

void parse_breakpoint_command(char *command, int command_length, struct file_and_line *file_and_line) {
	file_and_line->file_name = strtok(command, " ");
	char *line_number_ascii = strtok(NULL, " ");

	// printf("Command %s \n", command);
	// printf("Filename %s \n", file_and_line->file_name);
	// printf("Line %s \n", line_number_ascii);

	word line_number = my_atoi(line_number_ascii, strlen(line_number_ascii));
	if (line_number == -1) {
		printf("Invalid breakpoint command format. Should match 'b FILE_NAME LINE_NUMBER' \n");
	}
	// if invalid, this will be - 1
	file_and_line->line_number = line_number;
};
