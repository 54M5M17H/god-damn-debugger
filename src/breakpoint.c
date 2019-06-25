#include <breakpoint.h>
#include <gdd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>

long long unsigned int original_instruction;
long long unsigned int breakpoint_addr;

int set_breakpoint() {

	struct user_regs_struct registers;
	int res = ptrace(PTRACE_GETREGS, child_pid, NULL, &registers);
	if (res == -1) {
		perror("Fetch regs");
	}

	long long unsigned int instr = ptrace(PTRACE_PEEKTEXT, child_pid, registers.rip, 0);
	if (instr == -1) {
		perror("Fetch instr");
	}
	printf("RIP = 0x%08llx.  instr = 0x%08llx\n", registers.rip, instr);

	long long unsigned int breakpoint_addr = registers.rip + 3604;
	original_instruction = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)breakpoint_addr, 0);
	if (original_instruction == -1) {
		perror("PEEK");
	}
	printf("Original data at 0x%08llx: 0x%08llx\n", breakpoint_addr, original_instruction);

	long long unsigned int int_3_op = (original_instruction & 0xFFFFFF00) | 0xCC;
	res = ptrace(PTRACE_POKETEXT, child_pid, (void *)breakpoint_addr, (void *)int_3_op);
	if (res == -1) {
		perror("Adding bp");
	}
	printf("breakpoint added \n");

	long long unsigned int new_instruction = ptrace(PTRACE_PEEKTEXT, child_pid, (void *)breakpoint_addr, 0);
	if (new_instruction == -1) {
		perror("PEEK");
	}
	printf("Current data at 0x%08llx: 0x%08llx\n", breakpoint_addr, new_instruction);
}

void breakpoint_continue() {
	int res = ptrace(PTRACE_CONT, child_pid, NULL, NULL);
	if (res == -1) {
		perror("Continue");
	}
}

void resume_from_breakpoint() {
	printf("Resuming...\n");

	// replace instruction -- we might want to keep breakpoint there though
	int res = ptrace(PTRACE_POKETEXT, child_pid, breakpoint_addr, original_instruction);
	if (res == -1) {
		perror("Replace");
	}

	// back up one instruction
	struct user_regs_struct registers;
	res = ptrace(PTRACE_GETREGS, child_pid, 0, &registers);
	if (res == -1) {
		perror("Get registers");
	}

	registers.rip--;

	res = ptrace(PTRACE_SETREGS, child_pid, 0, &registers);
	if (res == -1) {
		perror("Backup");
	}

	one_step();

	// set_breakpoint();
	breakpoint_continue();
}

void one_step() {
	if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
		perror("step");
	}
}

void breakpoint_data() {
	struct user_regs_struct registers;
	int res = ptrace(PTRACE_GETREGS, child_pid, NULL, &registers);
	if (res == -1) {
		perror("Fetch regs");
	}

	long long unsigned int instr = ptrace(PTRACE_PEEKTEXT, child_pid, registers.rip, 0);
	if (instr == -1) {
		perror("Fetch instr");
	}
	printf("RIP = 0x%08llx.  instr = 0x%08llx\n", registers.rip, instr);
}

/**
 * struct user_regs_struct
{
  __extension__ unsigned long long int r15;
  __extension__ unsigned long long int r14;
  __extension__ unsigned long long int r13;
  __extension__ unsigned long long int r12;
  __extension__ unsigned long long int rbp;
  __extension__ unsigned long long int rbx;
  __extension__ unsigned long long int r11;
  __extension__ unsigned long long int r10;
  __extension__ unsigned long long int r9;
  __extension__ unsigned long long int r8;
  __extension__ unsigned long long int rax;
  __extension__ unsigned long long int rcx;
  __extension__ unsigned long long int rdx;
  __extension__ unsigned long long int rsi;
  __extension__ unsigned long long int rdi;
  __extension__ unsigned long long int orig_rax;
  __extension__ unsigned long long int rip;
  __extension__ unsigned long long int cs;
  __extension__ unsigned long long int eflags;
  __extension__ unsigned long long int rsp;
  __extension__ unsigned long long int ss;
  __extension__ unsigned long long int fs_base;
  __extension__ unsigned long long int gs_base;
  __extension__ unsigned long long int ds;
  __extension__ unsigned long long int es;
  __extension__ unsigned long long int fs;
  __extension__ unsigned long long int gs;
};
 */
