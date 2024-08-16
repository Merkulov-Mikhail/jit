#include "rex.h"


#define DST_POS dst_pos
#define DST 	dst

#define IR_ITEMS     ir_items
#define IR_ITEMS_POS pos

#define INSTRUCTION_POS instruction_number

#define NEW_IR(pos) \
	IR_ITEMS[IR_ITEMS_POS].pos_in_file = pos;

#define NEXT_IR() \
	IR_ITEMS_POS++;

#define NEXT_INSTRUCTION() \
	INSTRUCTION_POS++;

#define RESET_INSTRUCTION()\
	INSTRUCTION_POS = 0;

#define ADD_PREFIXES(first_byte, second_byte, third_byte, fourth_byte) \
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].prefixes[0] = first_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].prefixes[1] = second_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].prefixes[2] = third_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].prefixes[3] = fourth_byte;
	
#define ADD_OP_CODE(first_byte, second_byte, third_byte) \
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].op_code[0]  = first_byte;  \
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].op_code[1]  = second_byte; \
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].op_code[2]  = third_byte;

#define ADD_MOD_R(modR)\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].modR = modR;

#define ADD_SIB(SIB)\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].SIB = SIB;

#define ADD_DISPLACEMENT(first_byte, second_byte, third_byte, fourth_byte) \
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].displacement[0] = first_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].displacement[1] = second_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].displacement[2] = third_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].displacement[3] = fourth_byte;

#define ADD_IMMEDIATE(first_byte, second_byte, third_byte, fourth_byte) \
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].immediate[0] = first_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].immediate[1] = second_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].immediate[2] = third_byte;\
	IR_ITEMS[IR_ITEMS_POS].Instructions[INSTRUCTION_POS].immediate[3] = fourth_byte;

#define PUSH_REG_TO_STACK(reg) \
	IR_ITEMS[IR_ITEMS_POS]
	DST[DST_POS++] = 0x50 + reg;

#define PUSH_IMM_TO_STACK(imm) \
	DST[DST_POS++] = 0x86;\
	*(int*) (DST + DST_POS) = imm;\
	DST_POS += 4;


#define POP_REG_FROM_STACK(reg) \
	DST[DST_POS++] = 0x58 + reg;

#define REX_REG(flags) \
	DST[DST_POS++] = BASE_REX | flags;

#define MOV_QUAD_MEM_LEFT(dst_reg, src_reg, imm) \
	REX_REG(W_REX);\
	MOV_EXTD_MEM_LEFT(dst_reg, src_reg, imm);

#define MOV_QUAD_MEM_RIGHT(dst_reg, src_reg, imm)\
	REX_REG(W_REX);\
	MOV_EXTD_MEM_RIGHT(dst_reg, src_reg, imm);

#define MOV_EXTD_MEM_LEFT(dst_reg, src_reg, imm) \
	DST[DST_POS++] = 0x89;\
	ADD_TWO_REGISTERS(dst_reg, src_reg); \
	DST[DST_POS++] = ((1 << 7) + (1 << 6)) + (dst_reg << 3) + (src_reg);

#define MOV_EXTD_MEM_RIGHT(dst_reg, src_reg)\
	DST[DST_POS++] = 0x8b;\
	DST[DST_POS++] = ((1 << 7) + (1 << 6)) + (src_reg << 3) + (dst_reg);

#define ADD_TWO_REGISTERS(dst_reg, src_reg)\
	REX_REG(W_REX);\
	DST[DST_POS++] = 0x01;\
	
#define ENCODE_TWO_REG(first_reg, second_reg)\
	DST[DST_POS++] = ((1 << 7) + (1 << 6)) + (first_reg << 3) + (second_reg)

#define RETURN_COMMAND() \
	DST[DST_POS++] = 0xc3;

