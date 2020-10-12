#pragma once
#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAXCHAR 1000
#define TRUE 1
#define ZERO_ASCII 48
#define SEVEN_ASCII 55
#define MAX_LABEL_SIZE 31
#define ROW_LENGTH 2

typedef struct method_of_addressing
{

	int add_method_0;
	int add_method_1;
	int add_method_2;
	int add_method_3;

}method_of_addressing;

typedef struct possible_op {
	char* name;
	int how_many_operands;
	method_of_addressing src_address_method;
	method_of_addressing dst_address_method;
}possible_op;



typedef struct Data {
	short value;
	struct Data* next;
}data;
typedef struct label_table {
	char name[MAXCHAR];
	int address;
	int is_followed_by_instruction;
	int is_external;
	int is_entry;
	struct label_table* next;
}label_table;
typedef struct extern_list {
	struct extern_list* next;
	int address_of_use;
	char name[MAX_LABEL_SIZE];
}extern_list;

int is_valid_data(char* str);
void handle_label(char* label_name, int is_external, int is_followed_by_instruction, int row_number);
void handle_operation(int operation_index, char* string, int row_number);
char* trim_start(char* str);
char* trim_end(char* str);
char* trim_string(char* str);
char* read_next(char* string);
char* trim_quotation_marks(char* str);
char* read_data(char* string);
int check_if_not_reserved_string(char* string);
int is_valid_label(char* string);
void set_E(short* code_row);
void set_R(short* code_row);
void set_A(short* code_row);
void set_dst_address_method(short* code_row, int value);
void set_src_address_method(short* code_row, int value);
void set_opcode(short* code_row, int value);
void set_number_type_op(short* code_row, short value);
void insert_operation_and_operands_to_machine_code(int operation_index, char* string);
void set_src_register_type_op(short* code_row, short register_num);
void set_dst_register_type_op(short* code_row, short register_num);
int is_valid_number(char* str);
int get_num(char* str);
void set_label_type_op(short* code_row, struct label_table label);
void second_iteration();
label_table* get_label(char* label_name);
void first_iteration();
void write_output_files();
void write_entry_file();
void write_extern_file();
void write_object_file();
void add_to_extern_list(extern_list** head, int address, char* name);
void delete_extern_list(extern_list** head_ref);
long convert_bin_to_oct(short num);
void add_to_label_list(label_table** head, char* name, int address, int is_followed_by_instruction, int is_external);
void add_to_data_list(data** head, short value, int row_number);
void delete_data_list(data** head_ref);
void delete_label_list(label_table** head_ref);
int is_register(char* str);
void handle_data(char* str_starting_after_data, int row_number);
void add_IC_to_labels_followed_by_instructions();
void handle_entry(char* label_name);
#endif