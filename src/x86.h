#include "rex.h"

enum REGISTERS{
	RAX,
	RCX,
	RDX,
	RBX,
	RSP,
	RBP,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15
};


#define DST_POS dst_pos
#define DST 	dst

#define IR_ITEMS     ir_items
#define IR_ITEMS_POS ir_pos

#define INSTRUCTION_POS instruction_number

#define CONV_REG(reg) (reg > 7 ? reg - 8 : reg)

#define REX_REG(flags) \
	BASE_REX | flags

#define NEW_IR(pos) \
	IR_ITEMS->items[IR_ITEMS_POS]->pos_in_file = pos;

#define NEXT_IR() \
	IR_ITEMS_POS++;

#define NEXT_INSTRUCTION() \
	INSTRUCTION_POS++;

#define RESET_INSTRUCTION()\
	INSTRUCTION_POS = 0;

#define INC_BYTES(smth) \
	if (( smth ) != DEAD_VALUE) {\
		IR_ITEMS->items[IR_ITEMS_POS]->total_bytes++;\
		IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->total_bytes++;\
	}

#define PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, prefix)\
	if (prefix != DEAD_VALUE) {\
		if (first_reg > 7 && second_reg > 7) {\
			INSTRUCTION_ADD_PREFIXES(REX_REG(B_REX | R_REX) | prefix, DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		} else if (first_reg > 7) {\
			INSTRUCTION_ADD_PREFIXES(REX_REG(B_REX) | prefix, DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		} else if (second_reg > 7) {\
			INSTRUCTION_ADD_PREFIXES(REX_REG(R_REX) | prefix, DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		} else if (prefix != 0){ \
			INSTRUCTION_ADD_PREFIXES(prefix, DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		}\
	} else {\
		if (first_reg > 7 && second_reg > 7) {\
			INSTRUCTION_ADD_PREFIXES(REX_REG(B_REX | R_REX), DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		} else if (first_reg > 7) {\
			INSTRUCTION_ADD_PREFIXES(REX_REG(B_REX), DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		} else if (second_reg > 7) {\
			INSTRUCTION_ADD_PREFIXES(REX_REG(R_REX), DEAD_VALUE, DEAD_VALUE, DEAD_VALUE);\
		}\
	}


// DO NOT FORGET TO ADD IMMEDIATE CONST AFTERWARDS
#define MOD_R(first_reg, second_reg, is_memory, imm)\
	if (is_memory != DEAD_VALUE){ \
		if (imm != DEAD_VALUE) { INSTRUCTION_ADD_MOD_R((1 << 7) + INTERPRETATE_REGISTERS(first_reg, second_reg)); }\
		else { INSTRUCTION_ADD_MOD_R(INTERPRETATE_REGISTERS(first_reg, second_reg)); }\
	}\
	else {\
		INSTRUCTION_ADD_MOD_R((1 << 7) + (1 << 6) + INTERPRETATE_REGISTERS(first_reg, second_reg));\
	}

// I cannot handle this, so there will be no scaled indexes in my SIB byte and no index register
// so, always [REG1 + IMM], no [REG1 + REG2] [REG1 * IMM]
#define SIB(first_reg)\
	INSTRUCTION_ADD_SIB(INTERPRETATE_REGISTERS(0b0100, first_reg));

#define INTERPRETATE_REGISTERS(first_reg, second_reg) \
	(first_reg << 3) + (second_reg)

#define INSTRUCTION_ADD_PREFIXES(first_byte, second_byte, third_byte, fourth_byte) \
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->prefixes[0] = first_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->prefixes[1] = second_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->prefixes[2] = third_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->prefixes[3] = fourth_byte;\
	INC_BYTES(first_byte);\
	INC_BYTES(second_byte);\
	INC_BYTES(third_byte);\
	INC_BYTES(fourth_byte);

#define INSTRUCTION_ADD_OP_CODE(first_byte, second_byte, third_byte) \
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->op_code[0]  = first_byte;  \
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->op_code[1]  = second_byte; \
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->op_code[2]  = third_byte;  \
	INC_BYTES(first_byte);\
	INC_BYTES(second_byte);\
	INC_BYTES(third_byte);

#define INSTRUCTION_ADD_MOD_R(mod_R)\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->modR = mod_R;\
	INC_BYTES(mod_R);

#define INSTRUCTION_ADD_SIB(sib)\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->SIB = sib;\
	INC_BYTES(sib);

#define INSTRUCTION_ADD_DISPLACEMENT(displacement) \
	_INSTRUCTION_ADD_DISPLACEMENT((displacement & 0xff000000) >> 24, (displacement & 0x00ff0000) >> 16, (displacement & 0x0000ff00) >> 8, displacement & 0x000000ff);

#define _INSTRUCTION_ADD_DISPLACEMENT(first_byte, second_byte, third_byte, fourth_byte) \
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->displacement[3] = first_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->displacement[2] = second_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->displacement[1] = third_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->displacement[0] = fourth_byte;\
	INC_BYTES(first_byte);\
	INC_BYTES(second_byte);\
	INC_BYTES(third_byte);\
	INC_BYTES(fourth_byte);

#define INSTRUCTION_ADD_IMMEDIATE(imm) \
	_INSTRUCTION_ADD_IMMEDIATE((imm & 0xff000000) >> 24, (imm & 0x00ff0000) >> 16, (imm & 0x0000ff00) >> 8, imm & 0x000000ff);

#define _INSTRUCTION_ADD_IMMEDIATE(first_byte, second_byte, third_byte, fourth_byte) \
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->immediate[3] = first_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->immediate[2] = second_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->immediate[1] = third_byte;\
	IR_ITEMS->items[IR_ITEMS_POS]->Instructions[INSTRUCTION_POS]->immediate[0] = fourth_byte;\
	INC_BYTES(first_byte);\
	INC_BYTES(second_byte);\
	INC_BYTES(third_byte);\
	INC_BYTES(fourth_byte);

#define PUSH_REG_TO_STACK(reg) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, DEAD_VALUE);\
	INSTRUCTION_ADD_OP_CODE(0x50 + CONV_REG(reg), DEAD_VALUE, DEAD_VALUE);\
	NEXT_INSTRUCTION();

#define PUSH_IMM_TO_STACK(imm) \
	INSTRUCTION_ADD_OP_CODE(0x68, DEAD_VALUE, DEAD_VALUE);\
	INSTRUCTION_ADD_IMMEDIATE(imm);\
	NEXT_INSTRUCTION();

#define POP_REG_FROM_STACK(reg) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, DEAD_VALUE);\
	INSTRUCTION_ADD_OP_CODE(0x58 + CONV_REG(reg), DEAD_VALUE, DEAD_VALUE);\
	NEXT_INSTRUCTION();


// --------------------- MOV START ---------------------


#define MOV_EXTD_REG(dst_reg, src_reg) \
	INSTRUCTION_ADD_OP_CODE(0x89, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(dst_reg, src_ger, DEAD_VALUE, DEAD_VALUE);\
	NEXT_INSTRUCTION();

#define MOV_EXTD_MEM_LEFT(dst_reg, src_reg, imm) \
	INSTRUCTION_ADD_OP_CODE(0x89, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(src_reg, dst_reg, 1, imm);\
	if (imm != DEAD_VALUE) {\
		SIB(dst_reg);\
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	}\
	NEXT_INSTRUCTION();

#define MOV_EXTD_MEM_RIGHT(dst_reg, src_reg, imm)\
	INSTRUCTION_ADD_OP_CODE(0x8b, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(dst_reg, src_reg, 1, imm);\
	if (imm != DEAD_VALUE) {\
		SIB(src_reg);\
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	}\
	NEXT_INSTRUCTION();

#define MOV_QUAD_REG(dst_reg, src_reg) \
	PREFIXES_INCLUDING_NEW_REGS(dsr_reg, src_reg, REX_REG(W_REX));\
	MOV_EXTD_REG(CONV_REG(dst_reg), CONV_REG(src_reg));

#define MOV_QUAD_MEM_LEFT(dst_reg, src_reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(dst_reg, src_reg, REX_REG(W_REX));\
	MOV_EXTD_MEM_LEFT(CONV_REG(dst_reg), CONV_REG(src_reg), imm);

#define MOV_QUAD_MEM_RIGHT(dst_reg, src_reg, imm)\
	PREFIXES_INCLUDING_NEW_REGS(dst_reg, src_reg, REX_REG(W_REX));\
	MOV_EXTD_MEM_RIGHT(CONV_REG(dst_reg), CONV_REG(src_reg), imm);

// --------------------- MOV END ---------------------

// --------------------- ADD START ---------------------
#define ADD_IMM_EXT(reg, imm) \
	INSTRUCTION_ADD_OP_CODE(0x81, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(0, reg, DEAD_VALUE, DEAD_VALUE);\
	INSTRUCTION_ADD_IMMEDIATE(imm);\
	NEXT_INSTRUCTION();

#define ADD_IMM_EXT_NEW_REGS(reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, DEAD_VALUE);\
	ADD_IMM_EXT(CONV_REG(reg), imm);

#define ADD_IMM_QUAD(reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, REX_REG(W_REX));\
	ADD_IMM_EXT(CONV_REG(reg), imm);

#define ADD_REG_EXT_LEFT(first_reg, second_reg, is_memory, imm) \
	INSTRUCTION_ADD_OP_CODE(0x01, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(second_reg, first_reg, is_memory, imm);\
	if (is_memory != DEAD_VALUE){\
		SIB(first_reg);\
	}\
	if (imm != DEAD_VALUE) {\
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	}\
	NEXT_INSTRUCTION();

#define ADD_REG_QUAD_LEFT(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, REX_REG(W_REX));\
	ADD_REG_EXT_LEFT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

#define ADD_REG_EXT_LEFT_NEW_REGS(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, CONV_REG(second_reg), DEAD_VALUE);\
	ADD_REG_EXT_LEFT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

#define ADD_REG_EXT_RIGHT(first_reg, second_reg, is_memory, imm) \
	INSTRUCTION_ADD_OP_CODE(0x03, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(first_reg, second_reg, is_memory, imm);\
	if (is_memory != DEAD_VALUE){\
		SIB(second_reg);\
	}\
	if (imm != DEAD_VALUE) {\
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	}\
	NEXT_INSTRUCTION();

#define ADD_REG_QUAD_RIGHT(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, REX_REG(W_REX));\
	ADD_REG_EXT_RIGHT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

#define ADD_REG_EXT_RIGHT_NEW_REGS(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, DEAD_VALUE);\
	ADD_REG_EXT_RIGHT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

// --------------------- ADD END ---------------------

// --------------------- SUB START ---------------------
#define SUB_IMM_EXT(reg, imm) \
	INSTRUCTION_ADD_OP_CODE(0x81, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(5, reg, DEAD_VALUE, DEAD_VALUE);\
	INSTRUCTION_ADD_IMMEDIATE(imm);\
	NEXT_INSTRUCTION();

#define SUB_IMM_EXT_NEW_REGS(reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, DEAD_VALUE);\
	SUB_IMM_EXT(CONV_REG(reg), imm);

#define SUB_IMM_QUAD(reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, REX_REG(W_REX));\
	SUB_IMM_EXT(CONV_REG(reg), imm);

#define SUB_REG_EXT_LEFT(first_reg, second_reg, is_memory, imm) \
	INSTRUCTION_ADD_OP_CODE(0x29, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(second_reg, first_reg, is_memory, imm);\
	if (is_memory != DEAD_VALUE){\
		SIB(first_reg);\
	}\
	if (imm != DEAD_VALUE) {\
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	}\
	NEXT_INSTRUCTION();

#define SUB_REG_QUAD_LEFT(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, REX_REG(W_REX));\
	SUB_REG_EXT_LEFT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

#define SUB_REG_EXT_LEFT_NEW_REGS(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, DEAD_VALUE);\
	SUB_REG_EXT_LEFT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

#define SUB_REG_EXT_RIGHT(first_reg, second_reg, is_memory, imm) \
	INSTRUCTION_ADD_OP_CODE(0x2b, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(first_reg, second_reg, is_memory, imm);\
	if (is_memory != DEAD_VALUE){\
		SIB(second_reg);\
	}\
	if (imm != DEAD_VALUE) {\
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	}\
	NEXT_INSTRUCTION();

#define SUB_REG_QUAD_RIGHT(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, REX_REG(W_REX));\
	SUB_REG_EXT_RIGHT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

#define SUB_REG_EXT_RIGHT_NEW_REGS(first_reg, second_reg, is_memory, imm) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, DEAD_VALUE);\
	SUB_REG_EXT_RIGHT(CONV_REG(first_reg), CONV_REG(second_reg), is_memory, imm);

// --------------------- SUB END ---------------------

// --------------------- MUL START ---------------------

#define MUL_EXT(reg, is_memory, imm) \
	INSTRUCTION_ADD_OP_CODE(0xf7, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(4, reg, is_memory, imm);\
	if (is_memory != DEAD_VALUE){\
		SIB(reg);\
	}\
	if (imm != DEAD_VALUE) \
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	NEXT_INSTRUCTION();


#define MUL_QUAD(reg, is_memory, imm)\
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, REX_REG(W_REX));\
	MUL_EXT(CONV_REG(reg), is_memory, imm);


// --------------------- MUL END ---------------------

// --------------------- DIV START ---------------------

#define DIV_EXT(reg, is_memory, imm) \
	INSTRUCTION_ADD_OP_CODE(0xf7, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(6, reg, is_memory, imm);\
	if (is_memory != DEAD_VALUE){\
		SIB(reg);\
	}\
	if (imm != DEAD_VALUE) \
		INSTRUCTION_ADD_IMMEDIATE(imm);\
	NEXT_INSTRUCTION();


#define DIV_QUAD(reg, is_memory, imm)\
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, REX_REG(W_REX));\
	DIV_EXT(CONV_REG(reg), is_memory, imm);


// --------------------- DIV END ---------------------

// --------------------- CMP START ---------------------

#define CMP_IMM_EXT(reg, imm) \
	INSTRUCTION_ADD_OP_CODE(0x81, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(7, reg, DEAD_VALUE, DEAD_VALUE); \
	INSTRUCTION_ADD_IMMEDIATE(imm);\
	NEXT_INSTRUCTION();

#define CMP_IMM_QUAD(reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, REX_REG(W_REX));\
	CMP_IMM_EXT(CONV_REG(reg), imm);

#define CMP_IMM_EXT_NEW_REGS(reg, imm) \
	PREFIXES_INCLUDING_NEW_REGS(reg, DEAD_VALUE, 0);\
	CMP_IMM_EXT(CONV_REG(reg), imm);

#define CMP_REG_EXT(first_reg, second_reg) \
	INSTRUCTION_ADD_OP_CODE(0x39, DEAD_VALUE, DEAD_VALUE);\
	MOD_R(first_reg, second_reg, DEAD_VALUE, DEAD_VALUE); \
	NEXT_INSTRUCTION();

#define CMP_REG_QUAD(first_reg, second_reg) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, REX_REG(W_REX));\
	CMP_REG_EXT(CONV_REG(first_reg), CONV_REG(second_reg));

#define CMP_REG_EXT_NEW_REGS(first_reg, second_reg) \
	PREFIXES_INCLUDING_NEW_REGS(first_reg, second_reg, 0);\
	CMP_REG_EXT(CONV_REG(first_reg), CONV_REG(second_reg));

// --------------------- CMP END ---------------------

// --------------------- JUMPS START ---------------------

// It's quite tricky, how this works
// There will be documentation some day
#define JMP_IMM(imm) \
	INSTRUCTION_ADD_OP_CODE(0xE9, DEAD_VALUE, DEAD_VALUE);\
	_INSTRUCTION_ADD_IMMEDIATE(imm, imm, imm, imm);\
	NEXT_INSTRUCTION();

#define JCC(op_code, imm) \
	PREFIXES_INCLUDING_NEW_REGS(DEAD_VALUE, DEAD_VALUE, 0x0f);\
	INSTRUCTION_ADD_OP_CODE(op_code, DEAD_VALUE, DEAD_VALUE);\
	_INSTRUCTION_ADD_IMMEDIATE(imm, imm, imm, imm);\
	NEXT_INSTRUCTION();

#define JA_IMM(imm) \
	JCC(0x87, imm);

#define JAE_IMM(imm) \
	JCC(0x83, imm);

#define JB_IMM(imm) \
	JCC(0x82, imm);

#define JBE_IMM(imm) \
	JCC(0x86, imm);

#define JE_IMM(imm) \
	JCC(0x84, imm);

#define JNE_IMM(imm) \
	JCC(0x85, imm);

// --------------------- JUMPS END ---------------------

// --------------------- CALLS START ---------------------

#define CALL(imm) \
	INSTRUCTION_ADD_OP_CODE(0xE8, DEAD_VALUE, DEAD_VALUE);\
	_INSTRUCTION_ADD_IMMEDIATE(imm, imm, imm, imm);\
	NEXT_INSTRUCTION();

// --------------------- CALLS END ---------------------

#define RETURN_COMMAND() \
	INSTRUCTION_ADD_OP_CODE(0xc3, DEAD_VALUE, DEAD_VALUE);\
	NEXT_INSTRUCTION();
