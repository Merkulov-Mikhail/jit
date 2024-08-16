#ifndef _IR_H

#define _IR_H

#include <stdlib.h>
#include <assert.h>


const int MAX_ASSEMBLER_REPRESENTATION 	= 20;
const int DEAD_VALUE 					= -2;
const int IR_SIZE 						= 1000;
const int BIGGER_THAN_CHAR				= 256;

struct Instruction {
	int total_bytes;
	int prefixes[4];
	int op_code[3];
	int modR;
	int SIB;
	int displacement[4];
	int immediate[4];
};


struct IR_Item {
	int pos_in_file;
	int total_bytes;
	long long int actual_address;
	// one operation in SPU language can be rewritten as MAX_ASSEMBLER_REPRESENTATION assembler instructions
	Instruction* Instructions[MAX_ASSEMBLER_REPRESENTATION]; 
};

struct IR_Array {
	IR_Item** items;
	int size;
	int capacity;
};

Instruction* instructionCTOR();
void instructionDTOR(Instruction* p);

IR_Item* irItemCTOR();
void irItemDTOR(IR_Item* p);


Instruction* instructionCTOR() {
	Instruction* p = (Instruction*) calloc(sizeof(Instruction), 1);

	if (!p)
		return p;

	p->total_bytes = -1;

	for ( int i = 3; i > -1; i-- ) {
		p->prefixes[i]	   = BIGGER_THAN_CHAR; 
		p->displacement[i] = BIGGER_THAN_CHAR;
		p->immediate[i]	   = BIGGER_THAN_CHAR;
	}
	for ( int i = 2; i > -1; i-- ) {
		p->op_code[i] = BIGGER_THAN_CHAR;
	}
	p->SIB	= BIGGER_THAN_CHAR;
	p->modR	= BIGGER_THAN_CHAR;
	return p;
}

void instructionDTOR(Instruction* p) {

	assert(p);

	p->total_bytes = DEAD_VALUE;
	
	for ( int i = 3; i > -1; i-- ) {
		p->prefixes[i] 	   = DEAD_VALUE;
		p->displacement[i] = DEAD_VALUE;
		p->immediate[i]	   = DEAD_VALUE;
	}

	for (int i = 2; i > -1; i--) 
		p->op_code[i] = DEAD_VALUE;
	
	p->SIB 	= DEAD_VALUE;
	p->modR = DEAD_VALUE;

	free(p);

	return;
}

IR_Item* irItemCTOR() {
	IR_Item* p = (IR_Item*) calloc(sizeof(IR_Item), 1);

	if (!p)
		return p;

	p->pos_in_file   = 0;
	p->total_bytes   = 1;
	p->actual_address = 0;

	for ( int i = 0; i < MAX_ASSEMBLER_REPRESENTATION; i++ )
		p->Instructions[i] = instructionCTOR();

	return p;
}


void irItemDTOR(IR_Item* p) {

	assert(p);

	p->pos_in_file 	  = DEAD_VALUE;
	p->total_bytes	  = DEAD_VALUE;
	p->actual_address = DEAD_VALUE;

	for ( int i = 0; i < MAX_ASSEMBLER_REPRESENTATION; i++ )
		free(p->Instructions[i]);

	return;
}

IR_Array* irArrayCTOR() {
	IR_Array* p = (IR_Array*) calloc(sizeof(IR_Array), 1);

	if (!p)
		return p;

	p->capacity = IR_SIZE;
	p->size 	= 0;

	p->items = (IR_Item**) calloc(sizeof(IR_Item*), p->capacity);

	for (int i = 0; i < p->capacity; i++)
		p->items[i] = irItemCTOR();

	return p;
}

void irArrayDTOR(IR_Array* p){

	assert(p);

	for(int i = 0; i < p->capacity; i++)
		irItemDTOR(p->items[i]);

	free(p->items);
	p->size     = DEAD_VALUE;
	p->capacity = DEAD_VALUE;

	return;
}


IR_Array* irArrayREALLOC(IR_Array* p){

	assert(p);

	return p;
	
}

#endif
