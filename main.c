#define _CRT_SECURE_NO_DEPRECATE
#include "assembler.h"

struct possible_op possible_op_array[16] = {

		{"mov", 2, {1,1,1,1}, {0,1,1,1}},
		{"cmp", 2, {1,1,1,1}, {1,1,1,1}},
		{"add", 2, {1,1,1,1}, {0,1,1,1}},
		{"sub", 2, {1,1,1,1}, {0,1,1,1}},
		{"lea", 2, {0,1,0,0}, {0,1,1,1}},
		{"clr", 1, {0,0,0,0}, {0,1,1,1}},
		{"not", 1, {0,0,0,0}, {0,1,1,1}},
		{"inc", 1, {0,0,0,0}, {0,1,1,1}},
		{"dec", 1, {0,0,0,0}, {0,1,1,1}},
		{"jmp", 1, {0,0,0,0}, {0,1,1,0}},
		{"bne", 1, {0,0,0,0}, {0,1,1,0}},
		{"red", 1, {0,0,0,0}, {0,1,1,1}},
		{"prn", 1, {0,0,0,0}, {1,1,1,1}},
		{"jsr", 1, {0,0,0,0}, {0,1,1,0}},
		{"rts", 0, {0,0,0,0}, {0,0,0,0}},
		{"stop",0, {0,0,0,0}, {0,0,0,0}}
};
char* possible_instructions_array[4] = { ".string",".data",".entry",".extern" };
short code_array[MAXCHAR] = { 0 };

struct Data* data_list = NULL;
struct label_table* label_list = NULL;
extern_list* list_of_externs = NULL;
int DC;
int IC;
int L = 0;
int label_counter = 0;
char label_flag[MAXCHAR] = { 0 };
int error_counter = 0;



int main(int argc, char* argv[])
{
	char* fileName;
	int counter;
	/*
	if (argc == 1)
		printf("No file names given as arguments\n");
	if (argc >= 2)
	{
		for (counter = 1; counter < argc; counter++)
		{*/
	fileName = "program";//argv[counter];
	first_iteration(fileName);
	second_iteration(fileName);
	write_output_files(fileName);

	delete_data_list(&data_list);
	delete_extern_list(&list_of_externs);
	delete_label_list(&label_list);
	/*}
}*/
	return 0;
}
/*checking if the string is an assembler reserved word 
by comparing it to every one, firstly to a register name then to an instruction word and then to an opcode returning 1 if it is not reserved and zero otherwise*/
int check_if_not_reserved_string(char* string) {
	int i = 0;
	if (string[0] == 'r' && (string[1] >= ZERO_ASCII && string[1] <= SEVEN_ASCII)) {
		return 0;
	}
	if (strcmp(string, "extern") == 0 || strcmp(string, "entry") == 0 || strcmp(string, "data") == 0 || strcmp(string, "string") == 0) {
		return 0;
	}
	for (i = 0; i < 16; i++) {
		if (strcmp(possible_op_array[i].name, string) == 0) {
			return 0;
		}
	}
	return 1;
}
/*checking if the string is really a register reserved word by checking if the first letter is 'r' and the second one is an ASCII number character value between 0 and 7
or by checking if it is the other addrresing method by checking the existence of a '*' character followed by an 
'r' and a  ASCII number character value between 0 and 7 returning 1 if it is one and zero if not*/
int is_register(char* str) {
	if (str && ((str[0] == 'r' && (str[1] >= ZERO_ASCII && str[1] <= SEVEN_ASCII))
		|| (str[0] == '*' && str[1] == 'r' && (str[2] >= ZERO_ASCII && str[2] <= SEVEN_ASCII)))) {
		return 1;
	}
	return 0;
}
/*by calling a trim function at the start of a string and calling a trim function at the end i successfuly trim the unwanted characters and return the string*/
char* trim_string(char* str) {
	str = trim_start(str);
	str = trim_end(str);
	return str;
}
/*the trim start function removes the aforementioned characters from the start of string by looping on the string if encountering one it advances 
the string causing it to erase the character and returns it*/
char* trim_start(char* str) {
	while (str && (str[0] == ' ' || str[0] == '\t' || str[0] == '\n')) {
		str++;
	}
	return str;
}
/*the trim end function removes the aforementioned characters from the start of string by looping backwards if encountering one on the string and each time it encounter one it removes the last character
and returns it*/
char* trim_end(char* str) {
	while (str && (str[strlen(str) - 1] == ' ' || str[strlen(str) - 1] == '\t' || str[strlen(str) - 1] == '\n')) {
		str[strlen(str) - 1] = 0;
	}
	return str;
}
/*by calling the strtok function with the null value and passing the tab and space delimiter i successfully read the next word in the string containing the entire line 
and also trimming it of unwanted characters and returing it*/
char* read_next(char* string) {
	string = strtok(NULL, " \t");
	string = trim_string(string);
	return string;
}
/*by calling this function i loop on all the nodes in the extern linked list getting a reference to the head in the current variable and using 
it to treverse the list freeing every node im currently on and the getting the next one then finally setting the head reference NULL*/
void delete_extern_list(extern_list** head_ref)
{
	extern_list* current = *head_ref;
	extern_list* next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*head_ref = NULL;
}
/*by calling this function i loop on all the nodes in the data linked list getting a reference to the head in the current variable and using
it to treverse the list freeing every node im currently on and the getting the next one then finally setting the head reference NULL*/

void delete_data_list(data** head_ref)
{
	data* current = *head_ref;
	data* next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*head_ref = NULL;
}
/*by calling this function i loop on all the nodes in the label linked list getting a reference to the head in the current variable and using
it to treverse the list freeing every node im currently on and the getting the next one then finally setting the head reference NULL*/

void delete_label_list(label_table** head_ref)
{
	label_table* current = *head_ref;
	label_table* next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*head_ref = NULL;
}