// #include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
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
		printf("Launching child process: PID %i \n", getpid());
		return launch_program(args[1]);
	}

	// we're in the parent
	return attach_to_child(result);
}

int launch_program(char *program) {
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);
	return execl(program, program, NULL);
}

int attach_to_child(int childPID) {
	
}
