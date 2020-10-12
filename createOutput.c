#define _CRT_SECURE_NO_DEPRECATE
#include "assembler.h"
extern struct possible_op possible_op_array[16];
extern char* possible_instructions_array[4];
extern short code_array[MAXCHAR];

extern struct Data* data_list;
extern struct label_table* label_list;
extern extern_list* list_of_externs;
extern int DC;
extern int IC;
extern int L;
extern int label_counter;
extern char label_flag[MAXCHAR];
extern int error_counter;
extern char* fileName;
/* calls each of the write functions */
void write_output_files(char* fileName) {

	write_entry_file(fileName);
	write_extern_file(fileName);
	write_object_file(fileName);

}
/*by using the dynamic fileName passed as parameter to the function by the main function we construct the full file name by attaching the .ent in the end
with strncat and passing the variable storing the file name as a parameter of the fopen function write to it*/
void write_entry_file(char* fileName) {
	FILE* fptr;
	label_table* current = label_list;

	char fullFileName[MAXCHAR] = { 0 };
	strcpy(fullFileName, fileName);
	strncat(fullFileName, ".ent", 4);
	if ((fptr = fopen(fullFileName, "w")) == NULL) {
		printf("Error! writing %s file\n", fullFileName);
		exit(1);
	}
	/* by using the current we traverse the label list and print all the entry labels to the file*/
	while (current != NULL)
	{
		if (current->is_entry) {
			fprintf(fptr, "%s %d\n", current->name, current->address);
		}
		current = current->next;
	}
	/*closing the file we just wrote to*/
	fclose(fptr);
}
/**/
/*by using the dynamic fileName passed as parameter to the function by the main function we construct the full file name by attaching the .ext in the end
with strncat and passing the variable storing the file name as a parameter of the fopen function write to it*/
void write_extern_file(char* fileName) {
	FILE* fptr;
	label_table* current_label = label_list;
	char fullFileName[MAXCHAR] = { 0 };
	strcpy(fullFileName, fileName);
	strncat(fullFileName, ".ext", 4);

	if ((fptr = fopen(fullFileName, "w")) == NULL) {

		printf("Error! writing %s file\n", fullFileName);
		exit(1);
	}
	/* by using the current we traverse the label list and print all the extern labels to the file*/
	while (current_label != NULL)
	{
		if (current_label->is_external) {
			extern_list* current_extern = list_of_externs;

			while (current_extern != NULL)
			{
				if (strcmp(current_extern->name, current_label->name) == 0) {
					fprintf(fptr, "%s %d\n", current_extern->name, current_extern->address_of_use);
				}
				current_extern = current_extern->next;
			}
		};
		current_label = current_label->next;
	}
	/*by using the delete_extern_list we free all the allucated memory occupied by extern list nodes*/
	delete_extern_list(&list_of_externs);
	/*closing the file we just wrote to*/
	fclose(fptr);
}
/*by using the dynamic fileName passed as parameter to the function by the main function we construct the full file name by attaching the .ob in the end
with strncat and passing the variable storing the file name as a parameter of the fopen function write to it*/
void write_object_file(char* fileName) {
	FILE* fptr;
	int i;
	data* current = data_list;
	char fullFileName[MAXCHAR] = { 0 };
	strcpy(fullFileName, fileName);
	strncat(fullFileName, ".ob", 3);

	if ((fptr = fopen(fullFileName, "w")) == NULL) {
		printf("Error! writing %s file\n", fullFileName);
		exit(1);
	}
	/*first of all printing the IC and DC as required*/
	fprintf(fptr, "  %d %d\n", IC, DC);
	/*printing the all the addresses and the binary coding in octal with a loop by using the convert_bin_to_oct function*/
	for (i = 0; i < IC; i++) {
		fprintf(fptr, "%d %05li\n", i + 100, convert_bin_to_oct(code_array[i]));
	}
	i = 0;
	/*printing the data addresses and the binary coding in octal with a loop by using the convert_bin_to_oct function*/
	while (current != NULL)
	{
		short address;
		address = current->value;
		fprintf(fptr, "%d %05li\n", i + IC + 100, convert_bin_to_oct(address));
		current = current->next;
		i++;
	}
	/*closing the file we just wrote to*/
	fclose(fptr);
}

	/*by using 7 as mask i guarantee a 111 binary value and in a loop of five numbers guaranteeing 0 value if neccessary we build the octal
	value by using the bitwise & and getting every three bits and converting them to a real number to be added to the oct and then checking the next three
	and mulitplying by 10 to get the tens and hundreds*/
long convert_bin_to_oct(short num) {
	short mask = 7;
	short temp = num;
	long oct = 0;
	int mult = 1;
	int i;
	for (i = 0; i < 5; i++) {
		oct = oct + ((temp & mask) * mult);
		temp >>= 3;
		mult *= 10;
	}
	return oct;
}
