// #include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int launch_program(char *program);
int attach_to_child(int childPID);

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
		attach_to_child(result);
	}
}

int launch_program(char *program) {
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);

	char *const args[2] = {program, NULL};
	printf("Launching child process: PID %i \n", getpid());
	int res = execv(program, args);
	if (res == -1) {
		perror("exec");
	}
	printf("Done\n");
}

int attach_to_child(int childPID) {
	while (1) {
		int status;
		waitpid(childPID, &status, 0);
		if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
			printf("Child is stopped. Resuming...\n");
			ptrace(PTRACE_CONT, childPID, NULL, NULL);
		} else if (WIFEXITED(status)) {
			printf("Child exited.\n");
			printf("Terminating...\n");
			return 0;
		}
	}
}
