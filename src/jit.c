#include "config.h"
#include "ir.h"
// #include <sys/mman.h>
#include <windows.h>
#include <memoryapi.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


FILE* init_file( int argc, char* argv[] );
void* create_jit_buffer( uint64_t file_size );
uint64_t get_buffer_size( uint64_t file_size );
void load_jit_buffer( char* jit_buffer, IR_Array* ir_items );
int get_page_size();
int get_total_size( IR_Array* items );
int* create_mapping_array( IR_Array* ir_items, uint64_t size );
void actualize_addresses( IR_Array* ir_items, int* mapping );



int main( int argc, char* argv[] ){

	FILE* asm_file = init_file( argc, argv );

	if ( !asm_file ) {
		return -1;
	}
	// Reads from header size of the binary file
	uint64_t asm_file_size = 0;
	fseek( asm_file, 8, SEEK_SET );
	fread( & ( asm_file_size ), sizeof( uint64_t ), 1, asm_file );


	char* asm_file_data = ( char* ) calloc( sizeof( char ), asm_file_size + 10 );
	if ( !asm_file_data ) {
		printf( "Failed to initialize buffer to read assembler file" );
		return -1;
	}

	// skip the header and read whole binary file
	fseek( asm_file, ASM_HEADER_SIZE, SEEK_SET );
	fread( asm_file_data, sizeof( char ), asm_file_size, asm_file );

	IR_Array* ir_items = ir_array_CTOR();

	load_ir( ir_items, asm_file_data, asm_file_size );

	int resultingSize = get_total_size( ir_items );

	int* mapping_array = create_mapping_array( ir_items, resultingSize );
	char* jit_buffer = ( char* ) create_jit_buffer( resultingSize );

	actualize_addresses( ir_items, mapping_array );
	load_jit_buffer( jit_buffer, ir_items );

	#ifdef __linux__
		mprotect( jit_buffer, get_buffer_size( asm_file_size ), PROT_EXEC );
	#endif
	#ifdef _WIN32
		long unsigned ahahahah = 0;
		VirtualProtect( jit_buffer, get_buffer_size( resultingSize ), PAGE_EXECUTE, &ahahahah );
	#endif
	printf( "----%lld----\n", ( ( short ( * )() ) jit_buffer )() );

	#ifdef _WIN32
		VirtualProtect( jit_buffer, get_buffer_size( resultingSize ), ahahahah, &ahahahah );
	#endif

	ir_array_DTOR( ir_items );
	_aligned_free( jit_buffer );
	free		 ( asm_file_data );
	free		 ( mapping_array );

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

	// Check if the file was created via OWNER's assembly ( default - NumMeRiL )
	char owner[9] = {};
	fread( owner, sizeof( char ), 8, asm_file );
	if ( strcmp( owner, OWNER_NAME ) ) {
		printf( "Unknown format ( illegal owner name in header )\n" );
		return 0;
	}

	return asm_file;
}


// Assumes, that cursor of src_file is set on the first byte of the data
void* create_jit_buffer( uint64_t file_size ) {

	long page_size = get_page_size();

	uint64_t request_size = get_buffer_size( file_size );

	void* buf = _aligned_malloc( request_size, page_size );

	if ( !buf ) {
		printf( "A failure acquired while allocating data in aligned_alloc\n" );
		return 0;
	}

	memset( buf, 0xcc, request_size );

	return buf;
}


uint64_t get_buffer_size( uint64_t file_size ) {
	const long page_size = get_page_size();
	// !( !( file_size % page_size ) ) - returns 1, if page_size is not a divisor of file_size, othervise - 0
	return ( file_size / page_size + !( !( file_size % page_size ) ) ) * page_size;
}

int get_total_size( IR_Array* items ) {
	int result = 0;

	for ( int i = 0; i < items->size; i++ )
		result += items->items[i]->total_bytes;
	return result;
}


int get_page_size() {
	SYSTEM_INFO si;
	GetSystemInfo( &si );
	return si.dwPageSize;
}


//TODO: irArray realloc

void load_jit_buffer( char* jit_buffer, IR_Array* ir_items ) {

	assert( jit_buffer );
	assert( ir_items );

	uint64_t jit_pos = 0, ir_pos = 0;

	for ( ; ir_pos < ir_items->size; ir_pos++ ) {
		for ( int j = 0; j < MAX_ASSEMBLER_REPRESENTATION; j++ ) {
	// 		-----------prefixes-----------
			for ( int i = 0; i < 4; i++ )
				if ( ir_items->items[ir_pos]->Instructions[j]->prefixes[i] != DEAD_VALUE )
					jit_buffer[jit_pos++] = ir_items->items[ir_pos]->Instructions[j]->prefixes[i];
				else
					break;

	// 		-----------op code-----------
			for ( int i = 0; i < 3; i++ ){
				if ( ir_items->items[ir_pos]->Instructions[j]->op_code[i] != DEAD_VALUE )
					jit_buffer[jit_pos++] = ir_items->items[ir_pos]->Instructions[j]->op_code[i];
				else
					break;
			}

			if ( ir_items->items[ir_pos]->Instructions[j]->modR != DEAD_VALUE )
				jit_buffer[jit_pos++] = ir_items->items[ir_pos]->Instructions[j]->modR;

			if ( ir_items->items[ir_pos]->Instructions[j]->SIB != DEAD_VALUE )
				jit_buffer[jit_pos++] = ir_items->items[ir_pos]->Instructions[j]->SIB;

	// 		-----------displacement-----------
			for ( int i = 0; i < 4; i++ )
				if ( ir_items->items[ir_pos]->Instructions[j]->displacement[i] != DEAD_VALUE )
					jit_buffer[jit_pos++] = ir_items->items[ir_pos]->Instructions[j]->displacement[i];
				else
					break;

	// 		-----------immediate-----------
			for ( int i = 0; i < 4; i++ )
				if ( ir_items->items[ir_pos]->Instructions[j]->immediate[i] != DEAD_VALUE )
					jit_buffer[jit_pos++] = ir_items->items[ir_pos]->Instructions[j]->immediate[i];
				else
					break;
		}
	}

	return;
}

int* create_mapping_array( IR_Array* ir_items, uint64_t size ) {
	int* p = ( int* ) calloc( size, sizeof( int ) );

	for ( int i = 0; i < ir_items->size; i++ ) {
		p[ir_items->items[i]->pos_in_file] = ir_items->items[i]->actual_address;
	}

	return p;
}

void actualize_addresses( IR_Array* ir_items, int* mapping ) {
	int imm = 0;
	for ( int ir_pos = 0; ir_pos < ir_items->size; ir_pos++ ) {
		for ( int instruction_number = 0; instruction_number < MAX_ASSEMBLER_REPRESENTATION; instruction_number++ ) {
			if ( ir_items->items[ir_pos]->Instructions[instruction_number]->is_jmp_instruction != DEAD_VALUE ) {
				imm = ir_items->items[ir_pos]->Instructions[instruction_number]->immediate[0];
				ir_items->items[ir_pos]->Instructions[instruction_number]->immediate[0] = DEAD_VALUE;
				ir_items->items[ir_pos]->Instructions[instruction_number]->immediate[1] = DEAD_VALUE;
				ir_items->items[ir_pos]->Instructions[instruction_number]->immediate[2] = DEAD_VALUE;
				ir_items->items[ir_pos]->Instructions[instruction_number]->immediate[3] = DEAD_VALUE;
				INSTRUCTION_ADD_DISPLACEMENT( mapping[imm] - ir_items->items[ir_pos]->actual_address - ir_items->items[ir_pos]->total_bytes );
				ir_items->items[ir_pos]->Instructions[instruction_number]->total_bytes -= 4;
				ir_items->items[ir_pos]->total_bytes -= 4;

			}
		}
	}
}
