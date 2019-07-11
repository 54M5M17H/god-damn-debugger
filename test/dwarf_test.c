#define _GNU_SOURCE
#include "../lib/dwarf.h"
#include "../lib/libdwarf.h"
#include <fcntl.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t MAX_HASH_SIZE = 100;
int LARGEST_SUPPORTED_LINE_NUMBER_LENGTH = 6; // 6 digits

// TODO: de-allocation of dwarf -- see docs page 22

typedef struct hsearch_data *hash_table;

typedef struct file_and_line {
	char *file_name;
	Dwarf_Unsigned line_number;
} * File_And_Line;

// address -> line & file
hash_table address_to_file_and_line;

// file -> line -> address
hash_table file_and_line_to_address;

void insert(hash_table hash, char *key, void *val) {
	ENTRY insertion;
	insertion.key = key;
	insertion.data = val;
	ENTRY *retrieval;

	int res = hsearch_r(insertion, ENTER, &retrieval, hash);

	if (res == 0) {
		perror("Insert error");
		exit(0);
	}
}

ENTRY *find(hash_table hash, char *key) {
	ENTRY to_find;
	ENTRY *retrieval = malloc(sizeof(ENTRY));
	to_find.key = key;
	int res = hsearch_r(to_find, FIND, &retrieval, hash);
	if (res == 0) {
		perror("Find error");
		exit(0);
	}
	return retrieval;
}

void print_dwarf_err(Dwarf_Error *err, char *label) {
	printf("%s: %lld  %s0 \n", label, dwarf_errno(*err), dwarf_errmsg(*err));
}

Dwarf_Debug openFile(char *path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror("Opening file");
		return NULL;
	}

	Dwarf_Debug dbg = malloc(sizeof(Dwarf_Debug));
	Dwarf_Error err;

	int res = dwarf_init(fd, DW_DLC_READ, 0, 0, &dbg, &err);
	if (res != DW_DLV_OK) {
		printf("Error opening dwarf: %i \n", res);
		return NULL;
	}

	return dbg;
}

int get_die_info(Dwarf_Debug dbg, Dwarf_Die cu_die) {
	Dwarf_Error err;

	char *die_name;
	int res = dwarf_diename(
		cu_die,
		&die_name,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Getting die name");
		return -1;
	}

	printf("Name is: %s \n", die_name);

	Dwarf_Unsigned line_version;
	Dwarf_Small is_single_table;
	Dwarf_Line_Context line_context;

	res = dwarf_srclines_b(
		cu_die,
		&line_version,
		&is_single_table,
		&line_context,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Src lines context");
		return -1;
	}

	// inside file_and_line_to_address: file -> line -> address
	hash_table line_to_address = calloc(1, sizeof(hash_table));
	res = hcreate_r(MAX_HASH_SIZE, line_to_address);
	if (res == 0) {
		perror("Error initiliasing hash");
		return -1;
	}

	insert(file_and_line_to_address, die_name, line_to_address);

	Dwarf_Line *lines;
	Dwarf_Signed line_count;
	res = dwarf_srclines_from_linecontext(
		line_context,
		&lines,
		&line_count,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Src lines");
		return -1;
	}

	for (int i = 0; i < line_count; i++) {
		Dwarf_Unsigned line_num;
		res = dwarf_lineno(
			lines[i],
			&line_num,
			&err);

		if (res != DW_DLV_OK) {
			print_dwarf_err(&err, "Getting line number");
			return -1;
		}

		Dwarf_Addr *line_addr = malloc(sizeof(Dwarf_Addr));
		res = dwarf_lineaddr(
			lines[i],
			line_addr,
			&err);

		if (res != DW_DLV_OK) {
			print_dwarf_err(&err, "Getting line number addr");
			return -1;
		}

		char *line_num_key_res = l64a(line_num + 2); // this function is 2 off for whatever reason
		char *line_num_key = malloc(strlen(line_num_key_res) * sizeof(line_num_key_res[0]));
		strcpy(line_num_key, line_num_key_res); // and this because l64a seems to reuse same memory space

		char *line_addr_key = l64a((*line_addr) + 2);
		printf("line num is %lli, address is 0x%08llx, addr_key is %s, line_key is %s \n",
			   line_num, *line_addr, line_addr_key, line_num_key);
		insert(line_to_address, line_num_key, line_addr);
		File_And_Line fileAndLine = malloc(sizeof(File_And_Line));
		fileAndLine->line_number = line_num;
		fileAndLine->file_name = die_name;
		insert(address_to_file_and_line, line_addr_key, fileAndLine);
	}
}

int get_cu_die(Dwarf_Debug dbg) {
	Dwarf_Error err;
	Dwarf_Die no_die = 0, cu_die, child_die;

	int res = dwarf_siblingof(dbg, no_die, &cu_die, &err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Getting cu die");
		return -1;
	}

	get_die_info(dbg, cu_die);

	printf("\n\n");
	return 0;
}

int all_compilation_units(Dwarf_Debug dbg) {
	Dwarf_Unsigned cu_header_length, abbrev_offset, next_cu_header;
	Dwarf_Half version_stamp, address_size;
	Dwarf_Error err;

	int res;
	int counter = 0;
	while (1) {
		res = dwarf_next_cu_header(
			dbg,
			&cu_header_length,
			&version_stamp,
			&abbrev_offset,
			&address_size,
			&next_cu_header,
			&err);
		if (res != DW_DLV_OK) {
			break;
		}
		get_cu_die(dbg);
		counter++;
	}

	printf("Counter: %i \n", counter);

	if (res == DW_DLV_NO_ENTRY) {
		printf("Checked all CUs \n");
		return 0;
	}
	print_dwarf_err(&err, "Next CU Header");
	return -1;
}

int init_hashes() {
	address_to_file_and_line = calloc(1, sizeof(hash_table));
	file_and_line_to_address = calloc(1, sizeof(hash_table));
	int res = hcreate_r(MAX_HASH_SIZE, address_to_file_and_line);
	if (res == 0) {
		perror("Error initiliasing hash");
		return -1;
	}
	res = hcreate_r(MAX_HASH_SIZE, file_and_line_to_address);
	if (res == 0) {
		perror("Error initiliasing hash");
		return -1;
	}
}

int main() {
	int res = init_hashes();
	if (res == -1) {
		return -1;
	}
	Dwarf_Debug dbg = openFile("./target.o");
	if (!dbg) {
		printf("No Dwarf_Debug: exiting.");
		return -1;
	}

	all_compilation_units(dbg);

	Dwarf_Error err;

	res = dwarf_finish(dbg, &err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Dwarf finish");
		return -1;
	}

	printf("What line and file is address 0x00000609? \n");

	Dwarf_Addr addr = 0x00000609;
	char *addr_key = l64a(addr + 2);

	ENTRY *found = find(address_to_file_and_line, addr_key);
	File_And_Line data = (File_And_Line)found->data;
	printf("FileName: %s, lineNum: %08lld \n", data->file_name, data->line_number);

	printf("What address is line 2 of adder.c? \n");
	found = find(file_and_line_to_address, "adder.c");
	hash_table line_to_addr = (hash_table)found->data;

	Dwarf_Unsigned line_num = 2;
	char *line_key = l64a(line_num + 2);

	found = find(line_to_addr, line_key);
	Dwarf_Addr *found_addr = (Dwarf_Addr *)found->data;
	printf("Adder.c line 2 is at address 0x%08llx \n", *found_addr);

	// close(fd);
	printf("Closed \n");
}
