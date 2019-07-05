#include <sys/ptrace.h>
#include <sys/types.h>

typedef long long unsigned int word;
typedef struct user_regs_struct registers_struct;

pid_t child_pid;

// main.c
int MAX_INSTRUCTION_LEN;
int launch_program(char *program);

// debug.c
int start_debugger();
void handle_pause();
int get_command();

// utils.c
int my_atoi(char *str, int len);
void print_word(char *label, word to_print);

// breakpoint.c
void set_breakpoint_from_start();
void breakpoint_continue();
void set_breakpoint_at(word addr);

// ptrace_utils.c
int ptrace_run();
int ptrace_set_instruction(word address, word instruction);
int ptrace_step_back();
int ptrace_step_forward();
registers_struct *ptrace_get_registers();
int ptrace_set_registers(registers_struct *registers);
word ptrace_get_instruction(word address);
void ptrace_print_context();
