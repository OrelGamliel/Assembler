Assembler-
generating machine code files from a given assembly language file .as on an imaginary computer hardware

computer hardware

CONTAINING CPU, REGISTERS, RAM ,PART OF THE MEMORY IS USED AS STACK
this cpu has 8 registers named r0,r1,r2... each register is 15 bytes and also a PSW containing flags about the current CPU activity at each moment

this memory size is 4096 cells each 10 bytes big

this computer works only with negative or positive nuimbers no support for decimal points arithmatics are done in 2's complements also there is support for characters represented in ASCII

given an assembly code file generate three files 
.ob - containing the memory image of the machine code with two parts seperated with a space: 
  1.the address of the word in memory written in decimal (4 numbers ) with leading zeros 
  2.the contents of the word or instructionsin in octal (5 numbers ) 
.ext - containing the external labels and their corresponding addrresses in decimal 
.entry - containing the enternal labels and their corresponding addrresses in decimal

the next steps of linkage and loading are not covered here
