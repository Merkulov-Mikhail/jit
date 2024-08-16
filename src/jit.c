#include "config.h"
#include "ir.h"
#include "x86.h"
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


enum REGISTERS{
	RAX,
	RCX,
	RDX,
	RBX,
	RSP,
	RBP,
	RSI,
	RDI
};


const char REGISTER_TO_NAME_MAPPING[16] = {0, 3, 1, 2};

FILE* init_file( int argc, char* argv[] );
void* create_jit_buffer( uint64_t file_size );
uint64_t get_buffer_size( uint64_t file_size ); 
void load_jit_buffer( char* dst, const char* src, uint64_t buf_size );


int main(int argc, char* argv[]){
	printf("im here");

	FILE* asm_file = init_file( argc, argv );

	if ( !asm_file ) {
		return -1;
	}
	// Reads from header size of the binary file
	uint64_t asm_file_size = 0;
	fseek( asm_file, 8, SEEK_SET );
	fread( & ( asm_file_size ), sizeof( uint64_t ), 1, asm_file );

	printf("imhere");
	char* asm_file_data = ( char* ) calloc( sizeof( char ), asm_file_size - 16 );
	if ( !asm_file_data ) {
		printf( "Failed to initialize buffer to read assembler file" );
		return -1;
	}
	fseek( asm_file, 16, SEEK_SET );
	fread( asm_file_data, sizeof( char ), asm_file_size - 16, asm_file );


	char* jit_buffer = ( char* ) create_jit_buffer( asm_file_size );	

	IR_Array* ir_items = irArrayCTOR();

	load_jit_buffer( jit_buffer, asm_file_data, get_buffer_size( asm_file_size ) );
	for ( int i = 0; i < get_buffer_size( asm_file_size ); i++ )
		printf("%d\n", jit_buffer[i]);

	mprotect( jit_buffer, get_buffer_size( asm_file_size ), PROT_EXEC );


	printf("---%d---\n", (( short (*)()) jit_buffer )() );

	free( jit_buffer );
	free( asm_file_data );
	fclose( asm_file );
}


FILE* init_file( int argc, char* argv[] ) {

	assert( argv );

	if ( argc == 1 || argc > 2 ) {
		printf( "Usage:\njit <filename>\n" );
		return 0;
	}


	FILE* asm_file = fopen( argv[1], "rb" );
	if ( !asm_file ) {
		printf( "File not found\n" );
		return 0;
	}
	
	// Check if the file was created via OWNER's assembly (default - NumMeRiL)
	char owner[9] = {};
	fread( owner, sizeof( char ), 8, asm_file );
	if ( strcmp( owner, OWNER_NAME ) ) {
		printf( "Unknown format (illegal owner name in header)\n" );
		return 0;
	}

	return asm_file;
}


//Assumes, that cursor of src_file is set on the first byte of the data
void* create_jit_buffer( uint64_t file_size ) {

	long page_size = sysconf( _SC_PAGE_SIZE );

	uint64_t request_size = get_buffer_size( file_size );

	void* buf = aligned_alloc( page_size, request_size );

	if (!buf) {
		printf( "A failure acquired while allocating data in aligned_alloc\n" );
		return 0;
	}

	memset( buf, 0xc3, request_size );

	return buf;
}


uint64_t get_buffer_size( uint64_t file_size ) {
	const long page_size = sysconf( _SC_PAGE_SIZE );
	// !(!(file_size % page_size)) - returns 1, if page_size is not a divisor of file_size, othervise - 0
	return ( file_size / page_size + !(!( file_size % page_size )) ) * page_size;
}


void load_jit_buffer( char* dst, const char* src, uint64_t dst_size ) {

	assert(dst);
	assert(src);

	int  src_pos = 0;
	int  dst_pos = 0;
	char command = 0;
	int imm = 0;
	int reg = 0;
	int op_code = 0;

	while ( dst_pos < dst_size ) {
		command = src[src_pos++];

		switch ( command & ( R_BIT | I_BIT) ){
			case (R_BIT):
				reg = src[src_pos++];
				break;
			case (I_BIT):
				imm = *(double*) (src + src_pos);
				src_pos += 8;
				break;
			default:
				break;
		};

		op_code = command & ((1 << 5) - 1); 
		// clears first 3 bytes of command (aka operation code)
		// so now we have register in reg variable, immidiate const in imm
		switch ( op_code ) {
			case (PUSH):
				{
				if ( command & R_BIT ) {
					// PUSH_REG_TO_STACK(REGISTER_TO_NAME_MAPPING(reg));
				}
				else if ( command & I_BIT ) {
					PUSH_IMM_TO_STACK(imm);
				}
				break;
				}

			case (POP):
				{
				if (command & R_BIT) {
					// POP_REG_FROM_STACK(REGISTER_TO_NAME_MAPPING(reg));
				}
				else {
				
				}
				break;
				}
			case (ADD):
				{
				PUSH_REG_TO_STACK (RAX);
				
				}
			default:
				RETURN_COMMAND();


		};
	}

	return;
}
