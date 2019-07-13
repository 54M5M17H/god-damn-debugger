#include "../lib/dwarf.h"
#include "../lib/libdwarf.h"
#include <search.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>

typedef unsigned long long int word;
typedef unsigned long int half_word;
typedef struct user_regs_struct registers_struct;

typedef struct file_and_line {
	char *file_name;
	Dwarf_Unsigned line_number;
} * File_And_Line;

pid_t child_pid;

// main.c
int launch_program(char *program);

// debug_info.c
typedef struct hsearch_data *hash_table;

hash_table address_to_file_and_line;
hash_table file_to_file_lines;

hash_table init_hash(hash_table);
void insert(hash_table hash, char *key, void *val);
ENTRY *find(hash_table hash, char *key);
void print_dwarf_err(Dwarf_Error *err, char *label);
Dwarf_Debug debug_info_init(char *program_path);
void collate_src_file_info(Dwarf_Debug dbg);
char *get_src_file_name(Dwarf_Die cu_die);
struct source_lines_info *get_src_file_lines(Dwarf_Die cu_die);
Dwarf_Unsigned get_line_number(Dwarf_Line line);
Dwarf_Addr *get_line_address(Dwarf_Line line);
void collate_all_src_files(char *program);
char *long_to_hash_key(word num_to_hash);

void dwarf_test();

// debug.c
int start_debugger();
void handle_pause();

// utils.c
word my_atoi(char *str, int len);
void print_word(char *label, word to_print);

// breakpoint.c
void set_breakpoint_from_start();
void breakpoint_continue();
void set_breakpoint_at_address(word addr);

// ptrace_utils.c
int ptrace_run();
int ptrace_set_instruction(word address, word instruction);
int ptrace_step_back();
int ptrace_step_forward();
registers_struct *ptrace_get_registers();
int ptrace_set_registers(registers_struct *registers);
word ptrace_get_instruction(word address);
void ptrace_print_context();
