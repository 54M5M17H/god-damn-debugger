#include <gdd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>

// TODO: Handle errors from ptrace
// handle here or with user
// always exit ??

int ptrace_run() {
	int res = ptrace(PTRACE_CONT, child_pid, NULL, NULL);
	if (res == -1) {
		perror("Continue");
	}
	return res;
}

int ptrace_set_instruction(word address, word instruction) {
	// replace instruction at address
	int res = ptrace(PTRACE_POKETEXT, child_pid, (void *)address, (void *)instruction);
	if (res == -1) {
		perror("Setting instruction");
	}
	return res;
}

word ptrace_get_instruction(word address) {
	// print_word("address to get", address);
	// ptrace_print_ip_reg();
	word res = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)address, 0);
	if (res == -1) {
		perror("Peeking instruction");
	}
	return res;
}

int ptrace_step_back() {
	registers_struct *registers = ptrace_get_registers();
	registers->rip--;
	ptrace_set_registers(registers);
	free(registers);
	return 0;
}

int ptrace_step_forward() {
	int res = ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
	if (res == -1) {
		perror("Step forward");
	}
	return res;
}

registers_struct *ptrace_get_registers() {
	registers_struct *registers = malloc(sizeof(registers_struct));

	int res = ptrace(PTRACE_GETREGS, child_pid, 0, registers);
	if (res == -1) {
		perror("Get registers");
	}
	return registers;
}

int ptrace_set_registers(registers_struct *registers) {
	int res = ptrace(PTRACE_SETREGS, child_pid, 0, registers);
	if (res == -1) {
		perror("Set registers");
	}
	return res;
}

void ptrace_print_context(word adjustment) {
	registers_struct *registers = ptrace_get_registers();
	print_word("current ip reg", registers->rip);
	word next_instr = ptrace_get_instruction(registers->rip);
	print_word("next instruction", next_instr);
	free(registers);
	printf("\n");
}
