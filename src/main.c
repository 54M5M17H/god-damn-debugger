#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gdd.h>

int main(int argCount, char *args[]) {

	if (argCount <= 1) {
		puts("No program to run");
		return 0;
	}

	int result = fork();

	if (result == -1) {
		perror("Fork");
		return result;
	}

	if (result == 0) {
		launch_program(args[1]);
	} else {
		// we're in the parent
		child_pid = result;
		collate_all_src_files(args[1]);
		start_debugger();
	}
}

int launch_program(char *program) {
	ptrace(PTRACE_TRACEME, 0, 0, 0);

	// char *const args[2] = {program, NULL};
	printf("Launching child process: PID %i \n", getpid());
	// int res = execv(program, args);
	int res = execl(program, program, (void *)0);
	if (res == -1) {
		perror("exec");
	}
	printf("Done\n");
}
