#ifndef _IR_H

#define _IR_H

#include "x86.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


const int MAX_ASSEMBLER_REPRESENTATION 	= 20;
const int DEAD_VALUE 					= -2;
const int IR_SIZE 						= 1000;

const char REGISTER_TO_NAME_MAPPING[16] = {0, 3, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

struct Instruction {
	int is_jmp_instruction;
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

void load_ir( IR_Array* ir_items, const char* src, uint64_t src_size );


Instruction* instructionCTOR() {
	Instruction* p = (Instruction*) calloc(sizeof(Instruction), 1);

	if (!p)
		return p;

	p->total_bytes = 0;
	p->is_jmp_instruction = DEAD_VALUE;

	for ( int i = 3; i > -1; i-- ) {
		p->prefixes[i]	   = DEAD_VALUE;
		p->displacement[i] = DEAD_VALUE;
		p->immediate[i]	   = DEAD_VALUE;
	}
	for ( int i = 2; i > -1; i-- ) {
		p->op_code[i] = DEAD_VALUE;
	}
	p->SIB	= DEAD_VALUE;
	p->modR	= DEAD_VALUE;
	return p;
}

void instructionDTOR(Instruction* p) {

	assert(p);

	p->total_bytes = 0;

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


void instructionDump(Instruction* p) {

	assert(p);

	if ( p->total_bytes )
		printf("\ntotal bytes: %d\n", p->total_bytes);
	if ( p->prefixes[0] != DEAD_VALUE)
		for ( int i = 0; i < 4; i++ )
			if (p->prefixes[i] != DEAD_VALUE)
				printf("%d ", p->prefixes[i]);
			else
				break;

	if ( p->op_code[0] != DEAD_VALUE)
		for ( int i = 0; i < 3; i++ )
			if (p->op_code[i] != DEAD_VALUE)
				printf("%d ", p->op_code[i]);
			else
				break;

	if ( p->modR != DEAD_VALUE ) {
		printf("%d ", p->modR);
	}
	if ( p->SIB != DEAD_VALUE ) {
		printf("%d ", p->SIB);
	}

	if ( p->displacement[0] != DEAD_VALUE)
		for ( int i = 0; i < 4; i++ )
			if (p->displacement[i] != DEAD_VALUE)
				printf("%d ", p->displacement[i]);
			else
				break;

	if ( p->immediate[0] != DEAD_VALUE)
		for ( int i = 0; i < 4; i++ )
			if (p->immediate[i] != DEAD_VALUE)
				printf("%d ", p->immediate[i]);
			else
				break;
	return;
}


IR_Item* irItemCTOR() {
	IR_Item* p = (IR_Item*) calloc(sizeof(IR_Item), 1);

	if (!p)
		return p;

	p->pos_in_file   = 0;
	p->total_bytes   = 0;
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

void irItemDump(IR_Item* p) {

	assert(p);

	printf("\n----------IR----------\n%d\t%d\t%d\n", p->pos_in_file, p->total_bytes, p->actual_address);
	for (int i = 0; i < MAX_ASSEMBLER_REPRESENTATION; i++)
		instructionDump(p->Instructions[i]);

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

void irArrayDump(IR_Array* p) {

	assert(p);

	for (int i = 0; i < p->size; i++) {
		irItemDump(p->items[i]);
		printf("\n");
	}

}


void load_ir( IR_Array* ir_items, const char* src, uint64_t src_size ) {

	assert(ir_items);
	assert(src);

	int  ir_pos    = 0;
	int  src_pos   = 0;
	int  dst_pos   = 0;
	uint64_t  imm 	   = 0;
	int  reg 	   = 0;
	int  op_code   = 0;
	char command   = 0;
	int actual_pos = 0;

	int instruction_number = 0;

	while ( src_pos < src_size ) {
		ir_items->items[ir_pos]->pos_in_file = src_pos;
		ir_items->items[ir_pos]->actual_address = actual_pos;
		command = src[src_pos++];
 		op_code = command & ((1 << 5) - 1);


		switch ( command & ( R_BIT | I_BIT) ){
			case (R_BIT):
				reg = src[src_pos++];
				break;
			case (I_BIT):
				imm = *(uint64_t*) (src + src_pos);
				src_pos += 8;
				break;
			default:
				break;
		};

		// clears first 3 bytes of command (aka operation code)
		// so now we have register in reg variable, immidiate const in imm
		RESET_INSTRUCTION();
		switch ( op_code ) {
			case ( COMMANDS::PUSH ):
			{
				if ( command & R_BIT ) {
					PUSH_REG_TO_STACK(REGISTER_TO_NAME_MAPPING[reg]);
				}
				else if ( command & I_BIT ) {
					PUSH_IMM_TO_STACK(imm);
				}
				break;
			}

			case ( COMMANDS::POP ):
			{
				if ( command & R_BIT ) {
					POP_REG_FROM_STACK(REGISTER_TO_NAME_MAPPING[reg]);
				}
				break;
			}
			case ( COMMANDS::ADD ):
			{
				PUSH_REG_TO_STACK(RCX);
				MOV_QUAD_MEM_RIGHT(RCX, RSP, 0x8);
				ADD_REG_QUAD_LEFT(RSP, RCX, 1, 0x10);
				POP_REG_FROM_STACK(RCX);
				ADD_IMM_QUAD(RSP, 0x8);
				break;
			}
			case ( COMMANDS::SUB ):
			{
				PUSH_REG_TO_STACK(RCX);
				MOV_QUAD_MEM_RIGHT(RCX, RSP, 0x8);
				SUB_REG_QUAD_LEFT(RSP, RCX, 1, 0x10);
				POP_REG_FROM_STACK(RCX);
				ADD_IMM_QUAD(RSP, 0x8);
				break;
			}
			case ( COMMANDS::MUL ):
			{
				PUSH_REG_TO_STACK(RAX);
				PUSH_REG_TO_STACK(RDX);

				MOV_QUAD_MEM_RIGHT(RAX, RSP, 0x10);
				MUL_QUAD(RSP, 1, 0x18);
				MOV_QUAD_MEM_LEFT(RSP, RAX, 0x18)

				POP_REG_FROM_STACK(RDX);
				POP_REG_FROM_STACK(RAX);
				ADD_IMM_QUAD(RSP, 0x8);
				break;
			}
			case (COMMANDS::DIV ):
			{
				DIV_QUAD(RSP, 1, 0x0);
				ADD_IMM_QUAD(RSP, 0x8);
				break;
			}
			// in JCC instructions we can not save some registers values, so we polute R10 and R11
			// Be careful!
			#define JMP_PREPARATION()\
				POP_REG_FROM_STACK(R10);\
				POP_REG_FROM_STACK(R11);\
				CMP_REG_QUAD(R11, R10);\
				irItemDump(ir_items->items[ir_pos]);\

			case ( COMMANDS::JA ):
			{
				JMP_PREPARATION();
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JA_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::JAE ):
			{
				JMP_PREPARATION();
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JAE_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::JB ):
			{
				JMP_PREPARATION();
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JB_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::JBE ):
			{
				JMP_PREPARATION();
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JBE_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::JE ):
			{
				JMP_PREPARATION();
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JE_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::JNE ):
			{
				JMP_PREPARATION();
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JNE_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::JMP ):
			{
				ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction = 1;
				JMP_IMM(imm - 16);
				break;
			}
			case ( COMMANDS::CALL ):
			{
				break;
			}
			default:
				RETURN_COMMAND();
		}
		ir_items->size++;
		actual_pos += ir_items->items[ir_pos]->total_bytes;
		NEXT_IR();
	}
	return;
}



#endif
