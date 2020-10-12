
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



/*the first iteration of the assembler: the main loop is responsible for reading an entire line of strings until '\n' is reached, using fgets, 
from the .as file, the sub loop is responsible for reading each string ,using strtok with the space and tab delimiters, from the main loops 
entire line of strings, it runs until reaching the end of the line, analyzing each string, its uses,roles and validity in the assembler workflow 
while keeping an eye out for any and all errors ,if errors were found desplays an error message with the number of the line in the file.as it happend on.
after the main loop if errors were found the program will stop entirly and not continue the second iteration 
if not,we add the current IC to the lines which were followed by instructions, close the file and contiune with the program*/
void first_iteration(char* fileName) {
	int row_number = 0;
	char* string;
	FILE* fptr;
	char line_of_strings[MAXCHAR];
	char fullFileName[MAXCHAR] = { 0 };
	strcpy(fullFileName, fileName);
	strncat(fullFileName, ".as", 3);
	IC = 0;
	DC = 0;

	if ((fptr = fopen(fullFileName, "r")) == NULL) {
		printf("Error! opening %s file\n", fullFileName);
		/* Program exits if the file could not be opened.*/
		exit(1);
	}
	/*the main loop,responsible for getting the entire line of strings from the file.as */
	while (fgets(line_of_strings, MAXCHAR, fptr) != NULL) {
		row_number++;
		string = strtok(line_of_strings, " \t");
		string = trim_string(string);
		/*check if current string exists or if strings is a useless token meant to be skipped*/
		if (!*string || strcmp(string, ";") == 0) {
			continue;
		}
		/*usage of do and while so we will not advance string unnecessarily*/
		do
		{
			string = trim_string(string);
			/*checking if string is a label and a valid one using validation functions ,if it is label i use strcpy to keep the name in 
			label_flag variable for usage in the rest of the line*/
			if (string[strlen(string) - 1] == ':') {
				string[strlen(string) - 1] = 0;
				if (!check_if_not_reserved_string(string)) {
					error_counter++;
					printf("Error at row %d: reserved string used in label \n", row_number);
					continue;
				}
				if (!is_valid_label(string)) {
					error_counter++;
					printf("Error at row %d: illegal label\n", row_number);
					continue;
				}
				strcpy(label_flag, string);
			}
			/*checking if string has the first character of an instruction and if its a valid instruction by comparing it to the four possible instructions*/
			else if (string[0] == '.') {
				if ((strcmp(string, possible_instructions_array[0]) != 0) && (strcmp(string, possible_instructions_array[1]) != 0) && (strcmp(string, possible_instructions_array[2]) != 0) && (strcmp(string, possible_instructions_array[3]) != 0)) {
					printf("Error at row %d: syntax error found, wrong spelling of instructions\n", row_number);
					error_counter++;
					continue;
				}
				/* if the string is a string instruction i immediately read the next string checking if its a legal argument for string*/
				if (strcmp(string, ".string") == 0) {
					string = read_next(string);
					if (string[strlen(string) - 1] != '"' || string[0] != '"') {
						printf("Error at row %d: syntax error found, wrong data given to string instruction\n", row_number);
						error_counter++;
						continue;
					}
					string = trim_quotation_marks(string);
					/*insert label to label list if it was present in the line then emptying the label_flag variable with memset() for future usage*/
					if (*label_flag) {
						handle_label(label_flag, 0, 1, row_number);

						memset(label_flag, 0, MAXCHAR);
					}
					/*if string variable is carrying a value i add it to the data list character by character each time advancing 
					the DATA COUNTER (DC) then after it i add zero to the end
					symbolizing the end of the string*/
					while (*string) {
						add_to_data_list(&data_list, (short)string[0], row_number);
						string++;
						DC++;
					}
					add_to_data_list(&data_list, 0, row_number);
					DC++;
				}
				/*if the string is a data instruction, i get the line starting after the data keyword to catch every number characters
				checking the validity and handle the data using the function handle_data*/
				else if (strcmp(string, ".data") == 0) {

					int isValidData;
					char* line_starting_after_data_keyword;
					line_starting_after_data_keyword = read_data(string);
					isValidData = is_valid_data(line_starting_after_data_keyword);
					if (!isValidData) {
						printf("Error at row %d: data formatted wrong\n", row_number);
						error_counter++;
						continue;
					}

					if (*label_flag) {

						handle_label(label_flag, 0, 1, row_number);
						memset(label_flag, 0, MAXCHAR);
					}
					handle_data(line_starting_after_data_keyword, row_number);
				}
				/*if the string is a extern instruction, getting the next string in the line checking its validity as a label and entering it to the label list as an 
				extern label using the handle_label function and throwing a warning ,not an error, if a label was present in the line before that*/
				else if (strcmp(string, ".extern") == 0) {
					string = read_next(string);
					if (!check_if_not_reserved_string(string)) {
						error_counter++;
						printf("Error at row %d: reserved string used in label\n", row_number);
						continue;
					}
					if (!is_valid_label(string)) {
						error_counter++;
						printf("Error at row %d: illegal label\n", row_number);
						continue;
					}
					handle_label(string, 1, 2, row_number);

					if (*label_flag) {
						printf("WARNING at row %d: label is not needed before the extern command\n", row_number);
						memset(label_flag, 0, MAXCHAR);
					}
					/*checking if there is more data on the line as the extern instruction only takes one parameter ,if there is an error is printed*/
					string = read_next(string);
					if (string && *string) {
						error_counter++;
						printf("Error at row %d: extern only takes one parameter!\n", row_number);
						continue;
					}
				}
				/*an entry is ignored on the first iteration*/
				else if (strcmp(string, ".entry") == 0) {
					break;
				}
			}
			/*taking care of an opcode line: checking if it is a valid opcode by comparing it to all possible opcodes if not found an error is printed
			then handle the opration line with handle_operation*/
			else {
				int i;
				if (*label_flag) {

					handle_label(label_flag, 0, 0, row_number);
					memset(label_flag, 0, MAXCHAR);
				}

				for (i = 0; i < 16; i++) {
					if (strcmp(possible_op_array[i].name, string) == 0) {
						break;
					}
				}
				if (i == 16) {
					printf("Error at row %d: wrong opcode\n", row_number);
					error_counter++;
					break;
				}
				handle_operation(i, string, row_number);
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
	add_IC_to_labels_followed_by_instructions();

	fclose(fptr);
}

/*checking if the first character is not permitted ,if a not permitted character is found return 0 if all is well we enter the loop designed to keep a binary 
tracker turning on if it encounterd a permitted character and returning 0 if it encounted the same one again else zeroing the binary 
tracker, (in case of double '-' '+' or ',') and simultaneously checking if a character that is not a number and not a permitted
character is passed ,if it is return 0 else return 1 if all is well*/
int is_valid_data(char* str) {
	/*check comma legal*/
	int i = 0;
	int res = 1;

	int comma_indicator = 0;
	int hyphen_indicator = 0;
	int plus_indicator = 0;
	if (str[i] == ',' || (!isdigit(str[i]) && str[i] != '-' && str[i] != '+')) {
		return 0;
	}
	i++;
	while (str[i])
	{
		if ((str[i] == ',' && (comma_indicator == 1 || hyphen_indicator == 1 || plus_indicator == 1))
			|| (str[i] == '-' && hyphen_indicator == 1) || (str[i] == '+' && plus_indicator == 1)
			|| (str[i] != '+' && str[i] != ',' && str[i] != '-' && !isdigit(str[i]))) {
			res = 0;
			break;
		}
		if (str[i] == '-') {
			hyphen_indicator = 1;
			comma_indicator = 0;
			plus_indicator = 0;
		}
		if (str[i] == ',') {
			comma_indicator = 1;
			hyphen_indicator = 0;
			plus_indicator = 0;
		}
		if (str[i] == '+') {
			plus_indicator = 1;
			hyphen_indicator = 0;
			comma_indicator = 0;
		}
		else {
			comma_indicator = 0;
			hyphen_indicator = 0;
			plus_indicator = 0;
		}
		i++;
	}
	/*if a '-' character is found and the loop ends return 0, catching the end case that a number was not passed in the end*/
	if (comma_indicator == 1) {
		res = 0;
	}

	return res;
}
/*adding the label to the label list by using the node current and advancing it untill null ,we check if label already exists if it is found in lable list print 
an error, else add it as a node to the end with the appropriate parameters*/
void handle_label(char* label_name, int is_external, int is_followed_by_instruction, int row_number) {
	int address;

	label_table* current = label_list;

	while (current != NULL)
	{
		if (strcmp(current->name, label_name) == 0) {
			printf("Error at row %d: label already exists\n", row_number);
			error_counter++;
			continue;
		}
		current = current->next;
	}

	if (is_external) {
		address = 0;
	}
	else if (is_followed_by_instruction) {
		address = DC;
	}
	else {
		address = IC + 100;

	}
	add_to_label_list(&label_list, label_name, address, is_followed_by_instruction, is_external);
}
/*in a loop we check the line excluding every thing that is not a number character or a permitted character
if a number is encountered it tries to start turning it into number by using the  - '0' 
from the ASCII table, for example, decimal value of character 5 is 53 and 0 is 48. So 53 - 48 = 5 
and then turning on the is_number_exist binary tracker ,then it multiplies it by ten and adds the next number
(to get the tens ,hunderds and so forth..) until it reaches a ',' and turning off the is_number_exist binary tracker
and starting over with a new number and advancing the DC*/
void handle_data(char* line_starting_after_data_keyword, int row_number) {

	int i = 0;
	short num = 0;
	int is_number_exist = 0;
	int multi = 10;


	while (line_starting_after_data_keyword[i]) {

		if (isdigit(line_starting_after_data_keyword[i])) {
			if (is_number_exist == 0) {
				num = line_starting_after_data_keyword[i] - '0';
				is_number_exist = 1;
			}
			else {
				num = num * multi + (line_starting_after_data_keyword[i] - '0');
			}
		}
		else if (line_starting_after_data_keyword[i] == ',') {
			if (is_number_exist) {
				add_to_data_list(&data_list, num, row_number);
				is_number_exist = 0;
				DC++;
			}
		}
		/*when encountring a minus it multiplies it by minus to get a negative number*/
		else if (line_starting_after_data_keyword[i] == '-') {
			if (is_number_exist) {
				add_to_data_list(&data_list, num, row_number);
				is_number_exist = 0;
				DC++;
			}
			i++;
			num = -(line_starting_after_data_keyword[i] - '0');
			is_number_exist = 1;
		}
		i++;
	}
	/*catching the end case of a number being in preperation and loop ending not completing the number preperations*/
	if (is_number_exist) {
		add_to_data_list(&data_list, num, row_number);
		is_number_exist = 0;
		DC++;
	}
}
/*by using the opcode array containing all the possible addressing methods and how many and what kind of operands each opcodes operands
can be checked for validity if they fail an error will be printed*/
void handle_operation(int operation_index, char* string, int row_number) {

	int count = 1;
	int register_count = 0;
	
	int dst_condition_0, dst_condition_1, dst_condition_2, dst_condition_3;
	int src_condition_0, src_condition_1, src_condition_2, src_condition_3;
	/*currOperation holds each opcode information*/
	possible_op currOperation = possible_op_array[operation_index];
	/*curr_src_adrs_mthd holds the source addres method and curr_dst_adrs_mthd holds the destination addres method*/
	method_of_addressing curr_src_adrs_mthd = currOperation.src_address_method;
	method_of_addressing curr_dst_adrs_mthd = currOperation.dst_address_method;
	/*if the current opcode takes any source methods we start addressing the next operand*/
	if (curr_src_adrs_mthd.add_method_0 || curr_src_adrs_mthd.add_method_1 || curr_src_adrs_mthd.add_method_2 || curr_src_adrs_mthd.add_method_3) {
		string = strtok(NULL, ",");
		string = trim_start(string);
		/*check if the operand is one of the available methods to call a operand if they fail every check we print an error*/
		src_condition_0 = (curr_src_adrs_mthd.add_method_0 ? (string && string[0] != '#') : TRUE);
		src_condition_1 = (curr_src_adrs_mthd.add_method_1 ? (string && !isalpha(string[0]) && check_if_not_reserved_string(string)) : TRUE);
		src_condition_2 = (curr_src_adrs_mthd.add_method_2 ? (string && string[0] != '*') : TRUE);
		src_condition_3 = (curr_src_adrs_mthd.add_method_3 ? (string && string[0] != 'r') : TRUE);		
		if (src_condition_0 && src_condition_1 && src_condition_2 && src_condition_3) {
			printf("Error at row %d: illegal operand or missing operand in relation to the op\n", row_number);
			error_counter++;
		}
		/*checks validity of number operand and register operand*/
		else {
			if (string && string[0] == '#' && !is_valid_number(++string)) {
				printf("Error at row %d: illegal value for number\n", row_number);
				error_counter++;
			}
			if (is_register(string)) {
				register_count++;
			}

		}

	}
	/*if the current opcode takes any destination methods we start addressing the next operand*/
	if (curr_dst_adrs_mthd.add_method_0 || curr_dst_adrs_mthd.add_method_1 || curr_dst_adrs_mthd.add_method_2 || curr_dst_adrs_mthd.add_method_3) {
		string = strtok(NULL, "\n");
		string = trim_start(string);
		dst_condition_0 = (curr_dst_adrs_mthd.add_method_0 ? (string && string[0] != '#') : TRUE);
		dst_condition_1 = (curr_dst_adrs_mthd.add_method_1 ? (string && !isalpha(string[0]) && check_if_not_reserved_string(string)) : TRUE);
		dst_condition_2 = (curr_dst_adrs_mthd.add_method_2 ? (string && string[0] != '*') : TRUE);
		dst_condition_3 = (curr_dst_adrs_mthd.add_method_3 ? (string && string[0] != 'r') : TRUE);
		if (dst_condition_0 && dst_condition_1 && dst_condition_2 && dst_condition_3) {
			printf("Error at row %d: illegal operand or missing operand in relation to the op\n", row_number);
			error_counter++;
		}
		else {
			if (string && string[0] == '#' && !is_valid_number(++string)) {
				printf("Error at row %d: illegal value for number\n", row_number);
				error_counter++;
			}
			if (is_register(string)) {
				register_count++;
			}
		}
	}
	/*count holds the binary lines taken by each opcode line*/
	count += currOperation.how_many_operands;
	/*if both are register they sit on the same binary word*/
	if (register_count == 2) {
		count--;
	}
	L += count;
}
/*advancing the current and each time it encounters a node that is_followed_by_instruction it adds the IC + 100
to its address so it will sit in a currect address in relation to the other addresses*/
void add_IC_to_labels_followed_by_instructions() {
	label_table* current = label_list;

	while (current != NULL)
	{
		if (current->is_followed_by_instruction == 1) {
			current->address += IC + 100;
		}
		current = current->next;
	}
}
/* checking if a label is valid by making sure if it adheres the label rules*/
int is_valid_label(char* string) {
	int i = 1;
	if (strlen(string) > MAX_LABEL_SIZE) {
		return 0;
	}
	if (!isalpha(string[0])) {
		return 0;
	}
	while (string[i]) {
		if (!isalpha(string[i]) && !isdigit(string[i])) {
			return 0;
		}
		i++;
	}
	return 1;
}
/* checking if a number is valid by making sure if it adheres the number rules*/
int is_valid_number(char* str) {
	int i = 0;
	int res = 1;
	str = trim_string(str);
	if (!isdigit(str[i]) && str[i] != '-' && str[i] != '+') {
		return 0;
	}
	i++;
	while (str[i])
	{
		if (!isdigit(str[i])) {
			res = 0;
			break;
		}
		i++;
	}
	return res;
}
/*by calling the strtok function with the null value and passing the tab and space delimiter i successfully read the next string in the string containing the entire line
and also trimming it of unwanted characters and returing it*/
char* read_data(char* string) {
	string = strtok(NULL, "\n");
	string = trim_string(string);
	return string;
}
/*removing any if at all quotations from the start and end of a string*/
char* trim_quotation_marks(char* str) {
	if (str && (str[0] == '"')) {
		str++;
	}
	if (str && (str[strlen(str) - 1] == '"')) {
		str[strlen(str) - 1] = 0;
	}
	return str;
}

/*by using the newNode i create a new node and add a data value to it and depending on the state of the list adding it to the end of it*/
void add_to_data_list(data** head, short value, int row_number)
{
	data* newNode, * temp;

	newNode = (data*)malloc(sizeof(data));
	if (newNode == NULL)
	{
		printf("Error at row %d: Unable to allocate memory in add_to_data_list\n", row_number);
	}
	else
	{
		newNode->value = value;
		newNode->next = NULL;
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

/*by using the newNode i create a new node and add label parameters to it and depending on the state of the list adding it to the end of it*/
void add_to_label_list(label_table** head, char* name, int address, int is_followed_by_instruction, int is_external)
{
	label_table* newNode, * temp;

	newNode = (label_table*)malloc(sizeof(label_table));
	if (newNode == NULL)
	{
		printf("Unable to allocate memory in add_to_label_list\n");
	}
	else
	{
		newNode->address = address;
		newNode->is_followed_by_instruction = is_followed_by_instruction;
		newNode->is_external = is_external;
		newNode->is_entry = 0;
		strcpy(newNode->name, name);
		newNode->next = NULL;
		temp = *head;
		/*if list is empty set the head as newNode*/
		if (*head == NULL) {
			*head = newNode;
		}
		else {
			/*by using the head i traverse as temp to the end of the list and add it next to the final node*/
			while (temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = newNode;
		}
	}
}
