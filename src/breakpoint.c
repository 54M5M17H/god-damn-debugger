#include <gdd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

hash_table breakpoint_instruction_store;

void init_breakpoint_store() {
	breakpoint_instruction_store = init_hash(breakpoint_instruction_store);
}

void store_breakpoint(word addr, word instruction) {
	word *persistent_instruction = malloc(sizeof(word));
	*persistent_instruction = instruction;
	char *addr_hash = long_to_hash_key(addr);
	insert(breakpoint_instruction_store, addr_hash, persistent_instruction);
}

word *retrieve_breakpoint_instruction(word addr) {
	char *addr_hash = long_to_hash_key(addr);
	ENTRY *entry = find(breakpoint_instruction_store, addr_hash);
	if (entry == NULL) {
		printf("No breakpoint found here \n");
		return NULL;
	}
	word *original_instruction = (word *)entry->data;
	return original_instruction;
}

void set_breakpoint_at(File_And_Line file_and_line) {
	ENTRY *entry = find(file_to_file_lines, file_and_line->file_name);
	if (entry == NULL) {
		printf("File '%s' doesn't exist \n", file_and_line->file_name);
		return;
	}

	hash_table file_lines = (hash_table)entry->data;
	char *line_num_key = long_to_hash_key(file_and_line->line_number);
	entry = find(file_lines, line_num_key);
	if (entry == NULL) {
		printf("Cannot set a breakpoint at line %lli of %s\n",
			   file_and_line->line_number, file_and_line->file_name);
		return;
	}
	word *addr = (word *)entry->data;
	set_breakpoint_at_address(*addr);
}

void set_breakpoint_at_address(word addr) {
	word existing_instruction = ptrace_get_instruction(addr);
	store_breakpoint(addr, existing_instruction);

	word int_3_op = (existing_instruction & 0xFFFFFFFFFFFFFF00) | 0xCC;
	ptrace_set_instruction(addr, int_3_op);
	return;
}

void breakpoint_continue() {
	// 1) find out where we are
	registers_struct *registers = ptrace_get_registers();
	word current_addr = registers->rip - 1;

	// 2) find original instruction
	word *original_instruction = retrieve_breakpoint_instruction(current_addr);
	if (original_instruction == NULL) {
		return;
	}

	// 3) put back original instruction
	ptrace_set_instruction(current_addr, *original_instruction);

	// 4) step back
	ptrace_step_back();

	// 5) Step forward
	ptrace_step_forward();
	int status;
	waitpid(child_pid, &status, 0);
	if (!WIFSTOPPED(status) || !WSTOPSIG(status) == SIGTRAP) {
		exit(0);
	}

	// 6) Put breakpoint instruction back
	set_breakpoint_at_address(current_addr);

	// 7) run
	ptrace_resume();

	free(registers);
}

// open question: how to record breakpoints
// requirements:
// address -> instruction

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
