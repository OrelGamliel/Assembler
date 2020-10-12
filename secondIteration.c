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

/*in the second iteration we use a loop and subloop similiar to the first iteration but each time we encounter an entry instruction we search 
the label in the label list if found we instantiate the entry property of that same node else we print an error and in opcode lines we build the the 
binary code in the code array variable and advance the IC counter. if errors were found we stop here*/
void second_iteration(char* fileName) {
	char str[MAXCHAR];
	FILE* fptr;
	int row_number = 0;
	char fullFileName[MAXCHAR] = { 0 };
	strcpy(fullFileName, fileName);
	strncat(fullFileName, ".as", 3);
	IC = 0;
	L = 0;
	if ((fptr = fopen(fullFileName, "r")) == NULL) {
		printf("Error! opening %s file\n", fullFileName);

		exit(1);
	}
	while (fgets(str, MAXCHAR, fptr) != NULL) {
		char* string = strtok(str, " \t");
		string = trim_string(string);
		if (!*string || strcmp(string, ";") == 0) {
			continue;
		}
		do
		{
			string = trim_string(string);

			if (string[strlen(string) - 1] == ':') {
				continue;
			}
			/*check if string has the first character of an entry instruction and then read the next string to recieve the label 
			and handle it with the handle_entry function*/
			else if (string[0] == '.') {
				if (strcmp(string, ".entry") == 0) {
					string = read_next(string);
					handle_entry(string);
					string = read_next(string);
					/*checking if there is more data on the line as the entry instruction only takes one parameter ,if there is, an error is printed*/
					if (string && *string) {
						error_counter++;
						printf("entry only takes one parameter!\n");
						continue;
					}
				}
				else {
					break;
				}
			}
			/*taking care of an opcode line: getting the index of the opcode to be used in the insert_operation_and_operands_to_machine_code function*/
			else {
				int i = 0;
				for (i = 0; i < 16; i++) {
					if (strcmp(possible_op_array[i].name, string) == 0) {
						break;
					}
				}
				insert_operation_and_operands_to_machine_code(i, string);
			}
			/*the subloop that gets the next string*/
		} while ((string = strtok(NULL, " \t")) != NULL);
		/*the INSTRUNCTION COUNTER advancement*/
		IC = IC + L;
		L = 0;

	}
	/*if errors were found stop the program*/
	if (error_counter != 0) {
		exit(1);
	}

	fclose(fptr);
}

/*handles the conversion of opcode lines to binary code */
void insert_operation_and_operands_to_machine_code(int operation_index, char* string) {

	int count = 1;
	short src_val = 0;
	short dst_val = 0;
	int is_src_register = 0;
	/*currOperation holds each opcode information*/
	possible_op currOperation = possible_op_array[operation_index];
	method_of_addressing curr_src_adrs_mthd = currOperation.src_address_method;
	method_of_addressing curr_dst_adrs_mthd = currOperation.dst_address_method;
	label_table* label;
	/*setting the opcode*/
	set_opcode(&code_array[IC], operation_index);
	/*setting the A value in the code array*/
	set_A(&code_array[IC]);
	/*if the current opcode takes any source methods we start addressing the next operand*/
	if (curr_src_adrs_mthd.add_method_0 || curr_src_adrs_mthd.add_method_1 || curr_src_adrs_mthd.add_method_2 || curr_src_adrs_mthd.add_method_3) {
		string = strtok(NULL, ",");
		string = trim_start(string);
		/*getting the currect value if the operand is a number value*/
		if (string[0] == '#') {
			set_src_address_method(&code_array[IC], 0);
			src_val = get_num(++string);
			set_number_type_op(&code_array[IC + count], src_val);
		}
		/*getting the currect value if the operand is a label*/
		else if (isalpha(string[0]) && check_if_not_reserved_string(string)) {
			set_src_address_method(&code_array[IC], 1);
			label = get_label(string);
			if (label == 0) {
				printf("label not found exception!\n");
				error_counter++;
			}
			else {
				set_label_type_op(&code_array[IC + count], *label);
				if (label->is_external)
					add_to_extern_list(&list_of_externs, (IC + 100 + count), label->name);
			}
		}
		/*getting the currect value if the operand is a indirect register*/
		else if (string[0] == '*') {
			set_src_address_method(&code_array[IC], 2);
			set_src_register_type_op(&code_array[IC + count], (string[1]) - '0');
		}
		/*getting the currect value if the operand is a direct register*/
		else if (string[0] == 'r') {
			set_src_address_method(&code_array[IC], 3);
			set_src_register_type_op(&code_array[IC + count], (string[1]) - '0');
		}
		/*checking if the operand was a register*/
		if (is_register(string)) {
			is_src_register = 1;
		}
		count++;
	}
	/*if the current opcode takes any destination methods we start addressing the next operand with the same checkups*/
	if (curr_dst_adrs_mthd.add_method_0 || curr_dst_adrs_mthd.add_method_1 || curr_dst_adrs_mthd.add_method_2 || curr_dst_adrs_mthd.add_method_3) {
		string = strtok(NULL, "\n");
		string = trim_start(string);

		if (string[0] == '#') {
			set_dst_address_method(&code_array[IC], 0);
			dst_val = get_num(++string);
			set_number_type_op(&code_array[IC + count], dst_val);

		}
		else if (isalpha(string[0]) && check_if_not_reserved_string(string)) {
			set_dst_address_method(&code_array[IC], 1);
			label = get_label(string);
			if (label == 0) {
				printf("label not found exception!\n");
				error_counter++;
			}
			else {
				set_label_type_op(&code_array[IC + count], *label);
				if (label->is_external)
					add_to_extern_list(&list_of_externs, (IC + 100 + count), label->name);
			}
		}
		else if (string[0] == '*') {
			set_dst_address_method(&code_array[IC], 2);
			if (is_src_register == 1) {
				count--;
			}
			set_dst_register_type_op(&code_array[IC + count], (string[1]) - '0');
		}
		else if (string[0] == 'r') {
			set_dst_address_method(&code_array[IC], 3);
			/*if both are register they sit on the same binary word*/
			if (is_src_register == 1) {
				count--;
			}
			set_dst_register_type_op(&code_array[IC + count], (string[1]) - '0');
		}
		count++;
	}
	L += count;
}

/*responsible for handling the entry lables. we use current to iterate on the lable list
and check if the label read is in the list if not we print an error of a not found entry label
else we change the property of the label node to instantiate true on is_entry*/
void handle_entry(char* label_name) {

	label_table* current = label_list;

	while (current != NULL)
	{
		if (strcmp(current->name, label_name) == 0) {
			break;
		};
		current = current->next;
	}

	if (current == NULL) {
		printf("label declaration not found for entry\n");
		error_counter++;
		return;
	}
	if (current->is_external) {
		printf("label can not be entry because it is in use as an external label\n");
		error_counter++;
		return;
	}
	current->is_entry = 1;
}
/* with the bitwise operator '|' change the rightmost bit to 1 on the current code_row*/
void set_E(short* code_row) {
	short mask = 1;
	*code_row = *code_row | mask;
}
/* with the bitwise operator '|' change the first from last bit to 1 by moving the mask 1 times right*/
void set_R(short* code_row) {
	short mask = 1;
	*code_row = *code_row | mask << 1;
}
/* with the bitwise operator '|' change the second from last bit to 1 by moving the mask 2 times right*/
void set_A(short* code_row) {
	short mask = 1;
	*code_row = *code_row | mask << 2;
}
/* with the bitwise operator '|' change the destination address methods corresponding bit to on by passing the value needed to reach that specific bit*/
void set_dst_address_method(short* code_row, int value) {
	short mask = 1;
	*code_row = *code_row | mask << (3 + value);
}
/* with the bitwise operator '|' change the source address methods corresponding bit to on by passing the value needed to reach that specific bit*/
void set_src_address_method(short* code_row, int value) {
	short mask = 1;
	*code_row = *code_row | mask << (7 + value);
}
/* by adding the bit representation of the opcode number to the eleventh place in the code row we successfuly represent the number of the opcode used*/
void set_opcode(short* code_row, int value) {
	*code_row = (*code_row >> 4) + (value << 11);
}

void set_number_type_op(short* code_row, short value) {
	*code_row = value << 3;
	set_A(code_row);
}

void set_label_type_op(short* code_row, label_table label) {
	if (label.is_external) {
		set_E(code_row);
	}
	else {
		*code_row = label.address << 3;
		set_R(code_row);
	}
}

void set_dst_register_type_op(short* code_row, short register_num) {
	*code_row = *code_row + (register_num << 3);
	set_A(code_row);
}

void set_src_register_type_op(short* code_row, short register_num) {
	*code_row = *code_row + (register_num << 6);
	set_A(code_row);
}
/* we firstly check if the first character is a minus if it is we begin to creating the number with a minus by using the  - '0'
from the ASCII table, for example, decimal value of character 5 is 53 and 0 is 48. So 53 - 48 = 5
and then turning on the is_number_exist binary tracker ,then every number it gets in the loop is multiplied by ten and gets added the next number
(to get the tens ,hunderds and so forth..) until the string is finished*/
int get_num(char* str) {
	int i = 0;
	int num = 0;
	int is_number_exist = 0;
	int multi = 10;

	if (str[i] && str[i] == '-') {
		i++;
		num = -(str[i] - '0');
		is_number_exist = 1;
		i++;
	}
	while (str[i]) {
		if (isdigit(str[i])) {
			if (is_number_exist == 0) {
				num = str[i] - '0';
				is_number_exist = 1;
			}
			else {
				num = num * multi + (str[i] - '0');
			}
		}
		i++;
	}
	return num;
}
/*by recieving the desired label name and using the current we treverse the label list until we get a node with the same label name and return it*/
label_table* get_label(char* label_name) {
	label_table* current = label_list;

	while (current != NULL)
	{
		if (strcmp(current->name, label_name) == 0) {
			return current;
		}
		current = current->next;
	}
	return 0;
}

/*by using the newNode we create a new node and add extern properties to it and depending on the state of the list adding it to the end of it*/
void add_to_extern_list(extern_list** head, int address, char* name)
{
	extern_list* newNode, * temp;

	newNode = (extern_list*)malloc(sizeof(extern_list));
	if (newNode == NULL)
	{
		printf("Unable to allocate memory in add_to_extern_list\n");
	}
	else
	{
		newNode->address_of_use = address;
		newNode->next = NULL;
		strcpy(newNode->name, name);
		/*if list is empty set the head as newNode*/
		if (*head == NULL) {
			*head = newNode;
		}
		/*by using the head i traverse as temp to the end of the list and add it next to the final node*/
		else {
			temp = *head;

			while (temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = newNode;

		}
	}
}
