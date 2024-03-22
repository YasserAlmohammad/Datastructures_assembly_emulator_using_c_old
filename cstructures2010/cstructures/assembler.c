/*
read code from file, translate it into code list, execute the code instructions
code file is assumed to be formed, and properly generated
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
	MOVE,CMP,BRH,BEQ,OUT,ADD,SUB,MLT,DIV
} instruction_name;

#define name_size  10
#define operand_size  20
#define mem_size  64
#define registers_size 19

struct code_instruction{
	char name[name_size];
	char operand1[operand_size]; //parsed later.
	char operand2[operand_size];
	int line; //line number
};

//instruction node in the code list, double linked list definition
struct INode  {
	struct code_instruction instruction;
	struct INode* next;
	struct INode* prev;
};

struct Register{
	char* name;
	int value;
};

struct symbol{
	char name[name_size];
	struct INode* codePointer; //where does the label for instance points to in the code list.
};

struct symbol_node  {
	struct symbol sym;
	struct symbol_node* next;
	struct symbol_node* prev;
};

struct Register registers[registers_size];
struct INode* code; //pointer to head node of code.
int memory[mem_size];
struct symbol_node* symbol_table; //double linked list
int fatal_error = 0; //when its not 0 then indicates a fatal error that should prevent program execution
struct INode* currentExecution;

struct symbol_node* CreateSymbolNode(struct symbol s) {
	struct symbol_node* newNode = (struct symbol_node*)malloc(sizeof(struct symbol_node));
	newNode->sym = s;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}

//Creates a new Node and returns pointer to it. 
struct INode* CreateNewNode(struct code_instruction i) {
	struct INode* newNode= (struct INode*)malloc(sizeof(struct INode));
	newNode->instruction = i;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}
void executeCode(/*struct INode* node*/);

//Inserts a Node at head of the double linked list
void InsertAtHead(struct code_instruction i) {
	struct INode* newNode = CreateNewNode(i);
	if (code== NULL) {
		code = newNode;
		return;
	}
	code->prev = newNode;
	newNode->next = code;
	code = newNode;
}

//for the symbol table
void SymbolInsertAtHead(struct symbol s) {
	struct symbol_node* newNode = CreateSymbolNode(s);
	if (symbol_table == NULL) {
		symbol_table = newNode;
		return;
	}
	symbol_table->prev = newNode;
	newNode->next = symbol_table;
	symbol_table = newNode;
}

//Inserts a Node at the tail of the Double linked list
struct INode* InsertAtTail(struct code_instruction i) {
	struct INode* temp = code;
	struct INode* newNode = CreateNewNode(i);
	if (code == NULL) {
		code = newNode;
		return NULL;
	}
	while (temp->next != NULL) temp = temp->next; // Go To last Node
	temp->next = newNode;
	newNode->prev = temp;
	return newNode; //used by labels!
}

//print the instruction and line number.
void printInstruction(struct code_instruction i){
	printf("%s  ",i.name);
	if (i.operand1[0] != '\0')
		printf("%s",i.operand1);
	if (i.operand2[0]!='\0')
		printf(",%s", i.operand2);
}

//Prints all the elements in the linked list
void PrintCodeList() {
	struct INode* temp = code;
	printf("============code list content============\n");
	while (temp != NULL) {
		printInstruction(temp->instruction);
		temp = temp->next;
		printf("\n");
	}
	printf("===========content end===================\n");
}

void printSymbolTable(){
	struct symbol_node* temp = symbol_table;
	printf("============symbol table content============\n");
	while (temp != NULL) {
		printf("%s :", temp->sym.name);
		printInstruction(temp->sym.codePointer->instruction);
		temp = temp->next;
		printf("\n");
	}
	printf("===========content end===================\n");
}

//for parsing stage, op should either begin with $ # or be a register
int checkOperand(char* op){
	int res = 0;
	if (op == NULL)
		return 0;
	if (op[0] == '$')
		return 1;

	if (op[0] == '#')
		return 1;

	for (int i = 0; i < registers_size; i++){
		if (strcmp(op, registers[i].name) == 0)
			return 1;
	}
	return 0;
}
//read file content, containing instructions and store results in code list. scanning of input is required.
void parseFile(char* filename){
	FILE* file = NULL;
	file = fopen(filename, "r"); //or with safe reading as bellow
	//fopen_s(&file,filename, "r");
	if (!file){
		printf("File is not found\n");
		return;
	}

	char str[20];
	int icount = 0;
	while (fscanf(file, "%s", str) != EOF){
		//reading is guided by instruction, one operand, two, or none! considering input file is well formatted code.
		//MOVE,CMP,BRH,BEQ,OUT,ADD,SUB,MLT,DIV   if first command isnt one of those, then it is a label.
		if (strcmp(str, "MOV")==0){  //we can allow for spaces between , and operands, by creating more cases
			char part2[operand_size*2]; //the text part after the command
			char* op1;
			char* op2;
			fscanf(file, "%s",part2);
			op1 = strtok(part2, ",");
			op2 = strtok(NULL, ",");  //assume no space between operands.
			if ((checkOperand(op1)==0) || (checkOperand(op2) == 0)){
				fatal_error = 1;
				printf("Parsing Error: expecting operands after MOV at line :%d", icount);
				return;
			}
			struct code_instruction i;
			strcpy(i.name,str);
			strcpy(i.operand1,op1);
			strcpy(i.operand2,op2);
			i.line = icount;
			InsertAtTail(i);
		}

		else if (strcmp(str, "CMP") == 0){
			char part2[operand_size * 2]; //after the command
			char* op1;
			char* op2;
			fscanf(file, "%s", part2);
			op1 = strtok(part2, ",");
			op2 = strtok(NULL, ",");  //assume no space between operands.
			if ((checkOperand(op1) == 0) || (checkOperand(op2) == 0)){
				fatal_error = 1;
				printf("Parsing Error: expecting operands after CMP at line :%d", icount);
				return;
			}
			struct code_instruction i;
			strcpy(i.name, str);
			strcpy(i.operand1, op1);
			strcpy(i.operand2, op2);
			i.line = icount;
			InsertAtTail(i);
		}

		else if (strcmp(str, "BRH") == 0){
			char part2[operand_size * 2]; //after the command
			int cread=fscanf(file, "%s", part2);
			if (cread == 0){
				fatal_error = 1;
				printf("Parsing Error: expecting operands after BRH at line :%d", icount);
				return;
			}
			struct code_instruction i;
			strcpy(i.name, str);
			strcpy(i.operand1, part2);
			for (int j = 0; j < operand_size; j++){
				i.operand2[j] = '\0';
			}
			i.line = icount;
			InsertAtTail(i);
		}
		else if (strcmp(str, "BEQ") == 0){
			char part2[operand_size * 2]; //after the command
			int cread=fscanf(file, "%s", part2);
			if (cread == 0){
				fatal_error = 1;
				printf("Parsing Error: expecting operands after BEQ at line :%d", icount);
				return;
			}
			struct code_instruction i;
			strcpy(i.name, str);
			strcpy(i.operand1, part2);
			for (int j = 0; j < operand_size; j++){
				i.operand2[j] = '\0';
			}
			i.line = icount;
			InsertAtTail(i);
		}
		else if (strcmp(str, "OUT") == 0){
			char part2[operand_size * 2]; //after the command
			int cread = fscanf(file, "%s", part2);
			if (cread == 0){
				fatal_error = 1;
				printf("Parsing Error: expecting operands after OUT at line :%d", icount);
				return;
			}
			struct code_instruction i;
			strcpy(i.name, str);
			strcpy(i.operand1, part2); //operand may contain one or more registers!
			for (int j = 0; j < operand_size; j++){
				i.operand2[j] = '\0';
			}
			i.line = icount;
			InsertAtTail(i);
		}
		else if ((strcmp(str, "MLT") == 0) || (strcmp(str, "DIV") == 0) || (strcmp(str, "SUB") == 0) || (strcmp(str, "ADD") == 0)){
			char part2[operand_size * 2]; //after the command
			int cread = fscanf(file, "%s", part2);
			if ((cread == 0) || (checkOperand(part2) == 0)){
				fatal_error = 1;
				printf("Parsing Error: expecting operands after %s at line :%d", str,icount);
				return;
			}
			struct code_instruction i;
			strcpy(i.name, str);
			strcpy(i.operand1, part2);
			for (int j = 0; j < operand_size; j++){
				i.operand2[j] = '\0';
			}
			i.line = icount;
			InsertAtTail(i);
		}
		else{ //label followed by : either directly connected to the word, or separated on its own.
			char* dots= strchr(str, ':');
			if (dots == NULL) { //means : is separated from the label, so read one more string and discard the string
				char part2[operand_size * 2]; //after the command
				for (int j = 0; j < operand_size; j++){
					part2[j] = '\0';
				}

				fscanf(file, "%s", part2); //part2 must contain : other wise error is thrown
				if ((part2[0]!=':') || (strlen(part2)>1)){ //means : is not in the begining of part2, : should be seperated from the following
					printf("Error, : is expected after label name[%s][line:%d]\n", str, icount);
					fatal_error = 1;
					return;
				}

				struct code_instruction i;
				strcpy(i.name, str); 
				i.name[strlen(str)] = '\0'; //append at the end of the name
				for (int j = 0; j < operand_size; j++){
					i.operand1[j] = '\0';
					i.operand2[j] = '\0';
				}
				i.line = icount;
				struct INode* tail = InsertAtTail(i);
				struct symbol s;
				strcpy(s.name, i.name);
				s.codePointer = tail; //reference label from symbol table.
				SymbolInsertAtHead(s);
			
			}
			else{//store label without the : 
				//so this section should only add the label to the symbol table
				struct code_instruction i;
				strncpy(i.name, str,strlen(str)-1); //just removed the last char.
				i.name[strlen(str)-1] = '\0'; //append at the end of the name
				for (int j = 0; j < operand_size; j++){
					i.operand1[j] = '\0';
					i.operand2[j] = '\0';
				}
				i.line = icount;
				struct INode* tail=InsertAtTail(i);
				struct symbol s;
				strcpy(s.name, i.name);
				s.codePointer = tail; //reference label from symbol table.
				SymbolInsertAtHead(s);
			}

		}
		icount++;
	}

	fclose(file);
}

void readMemory(char* filename){
		FILE* file = NULL;
		file = fopen(filename, "r"); 
		if (!file){
			printf("Memory File is not found\n");
			return;
		}

		int value;
		int mem_index = 0;
		while (fscanf(file, "%d", &value) != EOF){
			if (mem_index > mem_size){
				printf("memory file has more data than memory allocated\n");
				return;
			}
			memory[mem_index++] = value;	
		}
}

void printMemory(){
	printf("========== memory content ==========\n");
	for (int i = 0; i < mem_size; i++){
		printf("%d ", memory[i]);
	}
	printf("\n=====================================\n");

}

void printRegisters(){
	printf("========== registers content ==========\n");
	for (int i = 0; i < registers_size; i++)
		printf("%s : %d\n", registers[i].name, registers[i].value);
	printf("========================================\n");
}

/**
initialize register values and names.
*/
void initRegisters(){
	registers[0].name = "ACC"; //most used one.
	
	registers[1].name = "REG_A";
	registers[2].name = "REG_B";
	registers[3].name = "REG_C";
	registers[4].name = "REG_D";
	registers[5].name = "REG_E";
	registers[6].name = "REG_F";
	registers[7].name = "REG_G";
	registers[8].name = "REG_H";

	registers[9].name = "INPR1";
	registers[10].name = "INPR2";
	registers[11].name = "INPR3";
	registers[12].name = "INPR4";
	registers[13].name = "INPR5";
	registers[14].name = "OUTR1";
	registers[15].name = "OUTR2";
	registers[16].name = "OUTR3";
	registers[17].name = "OUTR4";
	registers[18].name = "OUTR5";
	for (int i = 0; i < registers_size; i++){
		registers[i].value = 0;
	}
}

int getRegisterValue(char* name){
	for (int i = 0; i < registers_size; i++){
		if (strcmp(name, registers[i].name) == 0)
			return registers[i].value;
	}
	printf("error register is expected but %s is found instead\n");
	fatal_error = 1;
	return 0;
}

void setRegisterValue(char* name,int value){
	for (int i = 0; i < registers_size; i++){
		if (strcmp(name, registers[i].name) == 0){
			registers[i].value = value;
			return;
		}
	}
	printf("error register is expected but %s is found instead\n");
	fatal_error = 1;
}

//get value of operand, whether it was register, memory address or # value
int getValue(char* operand){
	int value = 0;
	if (operand[0] == '#'){
		//remaining chars should be number
		char* temp = operand;
		temp++; //skip first char
		value = atoi(temp);
	}
	else if (operand[0] == '$'){
		//remaining chars should be number
		char* temp = operand;
		temp++;
		int address = atoi(temp);
		if (address > mem_size){
			fatal_error = 1;
			printf("Error, index out of memory range\n");
			return 0;
		}
		value = memory[address];
	}
	else{ //should be a register.
		value = getRegisterValue(operand);
	}

	return value;
}

/*
MOVE op1,op2  op1 cant be value, should be a register or memory location
 op2 can be value, register, memory location.
*/
void executeMOV(struct code_instruction i){
	int value = getValue(i.operand2); //value of op2.
	//if(fatal_error){return;} //abort
	if (i.operand1[0] == '$'){
		char* temp = i.operand1;
		temp++;
		int address = atoi(temp);
		memory[address]=value;
	}
	else{
		setRegisterValue(i.operand1, value);
	}
}
//MOVE, CMP, BRH, BEQ, OUT, ADD, SUB, MLT, DIV
void executeCMP(struct code_instruction i){
	int op2Val = getValue(i.operand2);
	int op1Val = getValue(i.operand1);
	if (op1Val == op2Val){
		setRegisterValue("ACC", 0);
	}
	else if (op1Val > op2Val){
		setRegisterValue("ACC", 1);
	}
	else
		setRegisterValue("ACC", -1);
}



//L10 means go to line 10 , Foo means go to label Foo
void executeBRH(struct code_instruction i){
	if (i.operand1[0] == 'L'){
		char* temp = i.operand1;
		temp++; //skip first char
		int line = atoi(temp);
		if (line != 0){ //means not a line number
			//go to instruction at line number and execute
			struct INode* targetLineNode = code;
			for (int j = 1; j < line; j++){
				if (targetLineNode == NULL){
					printf("Error in Label %s out of code range limits", i.operand1);
					fatal_error = 1;
					return;
				}
				currentExecution = targetLineNode->next;
			}
		//	executeCode(currentExecution); //loop
			return;
		}
	}
	 //branch to label, access symbol table and go directly to target!
	struct symbol_node* temp = symbol_table;
	while (temp != NULL){
		if (strcmp(temp->sym.name, i.operand1)==0){
			currentExecution=temp->sym.codePointer; //execute code.
			return;
		}
		temp = temp->next;
	}
	
}

void executeBEQ(struct code_instruction i){
	//get ACC value
	if (registers[0].value == 0) //if ACC is 0 then execute branch
		executeBRH(i);
}

void executeOUT(struct code_instruction i){
	//needs a special parsing!
	char* operand = strtok(i.operand1, ",");
	while ((operand != NULL)){
		if (strcmp(operand, "ALL") == 0){
			printMemory();
			printRegisters();
		}
		else if (strcmp(operand, "ALLM") == 0)
			printMemory();
		else
		if (strcmp(operand, "ALLR") == 0)
			printRegisters();
		else
			printf(" %d \n", getValue(operand));
		operand = strtok(NULL, ",");
	}
}

void executeADD(struct code_instruction i){
	int val = getValue(i.operand1);
	registers[0].value += val;
}

void executeSUB(struct code_instruction i){
	int val = getValue(i.operand1);
	registers[0].value -= val;
}

void executeMLT(struct code_instruction i){
	int val = getValue(i.operand1);
	registers[0].value *= val;
}

void executeDIV(struct code_instruction i){
	int val = getValue(i.operand1);
	if (val == 0){
		printf("fatal error: division by 0 not allowed at %s %d\n", i.name, i.line);
		return;
	}
	registers[0].value /= val;
}

/*
this function can be recursive when labels are used.
*/
void executeCode(){
	if (fatal_error!=0){
		printf("Error: Fatal Error, cant execute the program, parsing errors\n");
		return;
	}
	while (currentExecution != NULL){ //MOVE, CMP, BRH, BEQ, OUT, ADD, SUB, MLT, DIV
		if (strcmp(currentExecution->instruction.name, "MOV") == 0)
			executeMOV(currentExecution->instruction);
		else
		if (strcmp(currentExecution->instruction.name, "CMP") == 0)
			executeCMP(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "BRH") == 0)
			executeBRH(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "BEQ") == 0)
			executeBEQ(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "OUT") == 0)
			executeOUT(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "ADD") == 0)
			executeADD(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "SUB") == 0)
			executeSUB(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "MLT") == 0)
			executeMLT(currentExecution->instruction);
		else if (strcmp(currentExecution->instruction.name, "DIV") == 0)
			executeDIV(currentExecution->instruction);
		else{
			//check if label in symbol table, error otherwise
			struct symbol_node* temp = symbol_table;
			while (temp != NULL){
				if (strcmp(temp->sym.name, currentExecution->instruction.name) == 0){ //label
					break;
				}
				temp = temp->next;
			}
			if (temp == NULL){
				fatal_error = 1;
				printf("Error, Instruction not supported %s %d\n", currentExecution->instruction.name, currentExecution->instruction.line);
				return;
			}

		}
	//	printInstruction(currentExecution->instruction);
	//	printf("\n");
		//label, no exec!
		currentExecution = currentExecution->next;
	}
}

void FlowControl(char* codeFile,char* memFile){
	initRegisters();//must be called first
	parseFile(codeFile);
	PrintCodeList();
	readMemory(memFile);
	currentExecution = code;
	executeCode();
}

//must accept arguments as file name
//one argument containing code file
int main(int argc, char *argv[]){
//	char* codeFile = "code.txt";
	char* memFile="memory.txt";
	if (argc == 2)
		FlowControl(argv[1], memFile);
	else
		printf("Must supply one argument: the file name that contains the code");
	getchar();
}