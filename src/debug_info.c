#define _GNU_SOURCE
#include <fcntl.h>
#include <gdd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// TODO: DEALLOC EVERYTHING FROM DWARF -- see docs page 22

size_t MAX_HASH_SIZE = 100;

struct source_lines_info {
	Dwarf_Line *lines;
	Dwarf_Signed line_count;
};

hash_table init_hash(hash_table hash) {
	hash = calloc(1, sizeof(hash_table));
	if (hash == NULL) {
		perror("Init hash");
		exit(-1);
	}

	int res = hcreate_r(MAX_HASH_SIZE, hash);
	if (res == 0) {
		perror("Error initiliasing hash");
		exit(-1);
	}
	return hash;
};

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
};

ENTRY *find(hash_table hash, char *key) {
	ENTRY to_find;
	ENTRY *retrieval = malloc(sizeof(ENTRY));
	to_find.key = key;
	int res = hsearch_r(to_find, FIND, &retrieval, hash);
	if (res == 0) {
		return NULL;
	}
	return retrieval;
};

void print_dwarf_err(Dwarf_Error *err, char *label) {
	printf("%s: %lld  %s0 \n", label, dwarf_errno(*err), dwarf_errmsg(*err));
}

Dwarf_Debug debug_info_init(char *program_path) {
	int fd = open(program_path, O_RDONLY);
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

	// TODO: close(fd); somewhere else

	return dbg;
};

char *get_src_file_name(Dwarf_Die cu_die) {
	Dwarf_Error err;

	char *die_name;
	int res = dwarf_diename(
		cu_die,
		&die_name,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Getting die name");
		exit(-1);
	}
	char *src_file_name = malloc(strlen(die_name) * sizeof(char *));
	strcpy(src_file_name, die_name);
	return src_file_name;
}

struct source_lines_info *get_src_file_lines(Dwarf_Die cu_die) {
	Dwarf_Unsigned line_version;
	Dwarf_Small is_single_table;
	Dwarf_Line_Context line_context;
	Dwarf_Error err;

	int res = dwarf_srclines_b(
		cu_die,
		&line_version,
		&is_single_table,
		&line_context,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Src lines context");
		exit(-1);
	}

	Dwarf_Line *lines;
	Dwarf_Signed line_count;
	res = dwarf_srclines_from_linecontext(
		line_context,
		&lines,
		&line_count,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Src lines");
		exit(-1);
	}

	struct source_lines_info *src_file_lines = malloc(sizeof(struct source_lines_info));
	// NOTE: I THINK LINES WILL NEED TO BE REALLOCATED
	src_file_lines->lines = lines;
	src_file_lines->line_count = line_count;
	return src_file_lines;
}

Dwarf_Unsigned get_line_number(Dwarf_Line line) {
	Dwarf_Unsigned line_num;
	Dwarf_Error err;

	int res = dwarf_lineno(
		line,
		&line_num,
		&err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Getting line number");
		exit(-1);
	}

	return line_num;
}

Dwarf_Addr *get_line_address(Dwarf_Line line) {
	Dwarf_Addr *line_addr = malloc(sizeof(Dwarf_Addr));
	Dwarf_Error err;

	int res = dwarf_lineaddr(
		line,
		line_addr,
		&err);

	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Getting line number addr");
		exit(-1);
	}
	return line_addr;
}

char *long_to_hash_key(word num_to_hash) {
	char *local = l64a(num_to_hash + 2); // this function is 2 off for whatever reason
	char *key = malloc(strlen(local) * sizeof(char *));
	strcpy(key, local);
	return key;
}

void collate_src_file_info(Dwarf_Debug dbg) {
	Dwarf_Error err;
	Dwarf_Die cu_die;

	int res = dwarf_siblingof(dbg, 0, &cu_die, &err);
	if (res != DW_DLV_OK) {
		print_dwarf_err(&err, "Getting CU DIE");
		exit(-1);
	}

	char *file_name = get_src_file_name(cu_die);

	hash_table file_line_to_address;
	file_line_to_address = init_hash(file_line_to_address);
	insert(file_to_file_lines, file_name, file_line_to_address);

	struct source_lines_info *src_lines = get_src_file_lines(cu_die);
	printf("File: %s \n", file_name);

	for (int i = 0; i < src_lines->line_count; i++) {
		Dwarf_Line line = src_lines->lines[i];
		Dwarf_Unsigned line_num = get_line_number(line);
		Dwarf_Addr *line_addr = get_line_address(line);

		char *line_num_key = long_to_hash_key(line_num);
		char *line_addr_key = long_to_hash_key(*line_addr);

		insert(file_line_to_address, line_num_key, line_addr);

		File_And_Line fileAndLine = malloc(sizeof(File_And_Line));
		fileAndLine->line_number = line_num;
		fileAndLine->file_name = file_name;
		insert(address_to_file_and_line, line_addr_key, fileAndLine);

		printf("line num is %lli, address is 0x%08llx, addr_key is %s, line_key is %s \n",
			   line_num, *line_addr, line_addr_key, line_num_key);
	}
}

void collate_all_src_files(char *program) {
	// initialise global hashes
	address_to_file_and_line = init_hash(address_to_file_and_line);
	file_to_file_lines = init_hash(file_to_file_lines);

	// open file
	Dwarf_Debug dbg = debug_info_init(program);

	// iterate src files
	Dwarf_Unsigned cu_header_length, abbrev_offset, next_cu_header;
	Dwarf_Half version_stamp, address_size;
	Dwarf_Error err;
	int res;
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

		collate_src_file_info(dbg);
	}

	if (res != DW_DLV_NO_ENTRY) {
		print_dwarf_err(&err, "Next CU Header");
		exit(-1);
	}

	dwarf_finish(dbg, &err);

	dwarf_test();
};

// TODO: REMOVE
void dwarf_test() {
	printf("What line and file is address 0x00400b5c? \n");

	Dwarf_Addr addr = 0x00400b5c;
	char *addr_key = l64a(addr + 2);

	ENTRY *found = find(address_to_file_and_line, addr_key);
	File_And_Line data = (File_And_Line)found->data;
	printf("FileName: %s, lineNum: %08lld \n", data->file_name, data->line_number);

	printf("What address is line 2 of adder.c? \n");
	found = find(file_to_file_lines, "adder.c");
	hash_table line_to_addr = (hash_table)found->data;

	Dwarf_Unsigned line_num = 2;
	char *line_key = l64a(line_num + 2);

	found = find(line_to_addr, line_key);
	Dwarf_Addr *found_addr = (Dwarf_Addr *)found->data;
	printf("Adder.c line 2 is at address 0x%08llx \n", *found_addr);
}
