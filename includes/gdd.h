#include <sys/types.h>

// main.c
int MAX_INSTRUCTION_LEN;
int launch_program(char *program);

// debug.c
int debug_child();
void handle_break();

pid_t child_pid;