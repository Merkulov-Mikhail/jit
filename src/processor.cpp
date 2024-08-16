#include "TXLib.h"
#include "processor.h"
#include "config.h"
#include "stack.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>


const double PRECISION = 1000;
const int64_t RAM_SIZE = 1024 * 1024 * sizeof( elem_t );

#ifdef LOGGING
    #define ON_LOGGING(...) __VA_ARGS__
#else
    #define ON_LOGGING(...)
#endif

#define ADD_JMP_COMMAND( name, func ) \
    if ( command == (COMMANDS::name | I_BIT) ){\
        if ( command != ( COMMANDS::JMP | I_BIT ) ){\
            stackPop( stk, &second  );   \
            stackPop( stk, &first );     \
        }\
                                     \
        uint64_t pos = *( uint64_t* ) ( processor.cs + processor.ip );\
        \
        ON_LOGGING(printf("[%d] "#name" command, result -> ", step));\
        \
        if ( first func second ) {      \
            processor.ip = pos;      \
            ON_LOGGING(printf("jumped to %lld\n", pos));\
        }                            \
        else{                        \
            processor.ip += sizeof( uint64_t );\
            ON_LOGGING(printf("skipped jump to %lld\n", pos));\
        }                            \
    }                                \
    else

#define COMMAND_RAM_BIT( name, func ) \
    if ( command == ( name ) ){              \
        int64_t address = 0;\
\
        char reg = *( char* ) ( processor.cs + processor.ip );\
        bool res = 0;\
\
        switch ( reg ){\
            case 0:\
                    address = processor.rax;\
                    break;\
            case 1:\
                    address = processor.rbx;\
                    break;\
            case 2:\
                    address = processor.rcx;\
                    break;\
            case 3:\
                    address = processor.rdx;\
                    break;\
            default:\
                    printf( "[%d] ERROR: BAD REGISTER\n", step );\
                    res = 1;\
        }\
        if ( res )\
            break;\
\
        if ( address < 0 || address >= RAM_SIZE ){\
            printf( "[%d] ERROR: BAD ADDRESS %lld", step, address );\
            break;\
        }\
\
        func;\
\
        processor.ip++;\
        ON_LOGGING( printf( "[%d] PUSHED INTO STACK FROM RAM, ADDRESS FROM REGISTER\n", step, reg ) );\
    }\
    else


#define ONE_STACK_ELEMENT_COMMAND( name, func )\
    if ( command == name ){\
        stackPop( stk, &first );\
        double tmp = first / PRECISION;\
        stackPush( stk, ( elem_t ) ( func( tmp ) * PRECISION ) );\
                                                                 \
        ON_LOGGING(printf( "[%d] ADDED %.3lf TO STACK\n", step, func( tmp ) ));\
    } else


elem_t _add_( elem_t first, elem_t second );
elem_t _sub_( elem_t first, elem_t second );
elem_t _div_( elem_t first, elem_t second );
elem_t _mul_( elem_t first, elem_t second );

void push( stack* stk, elem_t* first, elem_t* second, elem_t ( *func ) ( elem_t, elem_t ) );

void drawRect(RGBQUAD* dc, int x, int y, int r, int g, int b);

void SPUCtor( SPU* spu );
void SPUDtor( SPU* spu );

int main(){
    FILE* file = fopen( ASSEMBLY_FILE, "rb" );

    if ( !file ){
        printf( "Error: no input file" );
        return -1;
    }

    struct stat buf = {};

    stat( ASSEMBLY_FILE, &buf );
    uint64_t fileSize = buf.st_size;

    int command   = 0;

    SPU processor = {};
    SPUCtor( &processor );
    stack* stk =            &processor.stk;
    stack* recursionStack = &processor.rs;

    RGBQUAD* dc = nullptr;

    fileSize = fread( processor.cs, sizeof( char ), fileSize, file );
    fclose( file );

    uint64_t step = 0;

    elem_t first = 0, second = 0;

    int hold_up = 0; // if window will be created
    while ( processor.ip < fileSize ){
        // stackDump(stk, 0);
        // littleDump(stk);
        // littleDump(recursionStack);

        command = *( unsigned char* ) ( processor.cs + processor.ip );

        ON_LOGGING(printf( "current step [%d] ( command_ID = %d, command_Address = %lld )\n", step, command, processor.ip ));

        processor.ip++;

        if      ( command == COMMANDS::ADD ){

            push( stk, &first, &second, _add_ );

            ON_LOGGING(printf( "[%d] ADDED %.3lf TO STACK ( %.3lf + %.3lf )\n", step, _add_( first, second ) / PRECISION, first / PRECISION, second / PRECISION ));
        }

        else if ( command == COMMANDS::SUB ){

            push( stk, &first, &second, _sub_ );
            ON_LOGGING(printf( "[%d] ADDED %.3lf TO STACK ( %.3lf - %.3lf )\n", step, _sub_( first, second ) / PRECISION, second / PRECISION, first / PRECISION ));
        }

        else if ( command == COMMANDS::MUL ){

            push( stk, &first, &second, _mul_ );
            ON_LOGGING(printf( "[%d] ADDED %.3lf TO STACK ( %.3lf * %.3lf )\n", step, _mul_( first, second ) / PRECISION, first / PRECISION, second / PRECISION ));
        }

        else if ( command == COMMANDS::DIV ){

            push( stk, &first, &second, _div_ );

            if ( first == 0 ){
                printf( "Error: zero division\n" );
                return -1;
            }
            else{
                ON_LOGGING(printf( "[%d] ADDED %.3lf TO STACK ( %.3lf / %.3lf )\n", step, _div_( first, second ) / PRECISION, second / PRECISION, first / PRECISION ));
            }
        }

        else if ( command == ( COMMANDS::PUSH | I_BIT ) ){

            double elem = *( double* ) ( processor.cs + processor.ip );

            stackPush( stk, ( elem_t ) elem * PRECISION );

            processor.ip += sizeof( double );
            ON_LOGGING(printf( "[%d] PUSHED %lf INTO STACK\n", step, elem ));
        }

        else if ( command == ( COMMANDS::PUSH | R_BIT ) ){

            char reg = *( char* ) ( processor.cs + processor.ip );
            bool res = 0;

            switch ( reg ){
                case 0:
                        stackPush( stk, ( elem_t ) processor.rax );
                        break;
                case 1:
                        stackPush( stk, ( elem_t ) processor.rbx );
                        break;
                case 2:
                        stackPush( stk, ( elem_t ) processor.rcx );
                        break;
                case 3:
                        stackPush( stk, ( elem_t ) processor.rdx );
                        break;
                default:
                        printf( "[%d] BAD REGISTER\n", step );
                        res = 1;
            }
            if ( res )
                break;

            processor.ip++;

            ON_LOGGING( printf( "[%d] PUSHED INTO STACK FROM %d REGISTER\n", step, reg ) );
        }
        else
        COMMAND_RAM_BIT( COMMANDS::PUSH | R_BIT | M_BIT, stackPush( stk,  processor.ram[address] ) )
        COMMAND_RAM_BIT( COMMANDS::POP  | R_BIT | M_BIT, stackPop(  stk, &processor.ram[address] ) )

        if ( command == ( COMMANDS::POP | I_BIT | M_BIT ) ){

            int64_t address = *( int64_t* ) ( processor.cs + processor.ip );

            if ( address < 0 || address >= RAM_SIZE ){
                printf( "[%d] ERROR: BAD ADDRESS %lld", step, address );
                break;
            }

            stackPop( stk, &( processor.ram[address] ) );

            processor.ip += sizeof( int64_t );

            ON_LOGGING( printf( "[%d] PUSHED INTO RAM FROM STACK, ADDRESS - %lld\n", step, address ) );
        }

        else if ( command == ( COMMANDS::PUSH | I_BIT | M_BIT ) ){

            int64_t address = *( int64_t* ) ( processor.cs + processor.ip );

            if ( address < 0 || address >= RAM_SIZE ){
                printf( "[%d] ERROR: BAD ADDRESS %lld", step, address );
                break;
            }

            stackPush( stk, processor.ram[address] );

            processor.ip += sizeof( int64_t );

            ON_LOGGING( printf( "[%d] PUSHED INTO RAM FROM STACK, ADDRESS - %lld\n", step, address ) );
        }

        else if ( command == ( COMMANDS::POP | R_BIT ) ){

            char reg = *( char* ) ( processor.cs + processor.ip );
            elem_t value = 0;
            bool res = 0;

            switch ( reg ){
                case 0:
                        stackPop( stk, &value );
                        processor.rax = value;
                        break;
                case 1:
                        stackPop( stk, &value );
                        processor.rbx = value;
                        break;
                case 2:
                        stackPop( stk, &value );
                        processor.rcx = value;
                        break;
                case 3:
                        stackPop( stk, &value );
                        processor.rdx = value;
                        break;
                default:
                        printf( "[%d] BAD REGISTER\n", step );
                        res = 1;
            }
            if ( res )
                break;

            ON_LOGGING(printf( "[%d] POPPED FROM STACK INTO %d REGISTER %lld\n", step, reg, value ));
            processor.ip++;
        }
        else
        ONE_STACK_ELEMENT_COMMAND( COMMANDS::SQRT, sqrt )
        ONE_STACK_ELEMENT_COMMAND( COMMANDS::SIN , sin  )
        ONE_STACK_ELEMENT_COMMAND( COMMANDS::COS , cos  )

        // else
       if ( command == COMMANDS::IN ){
            ON_LOGGING(printf( "USER_INPUT: " ));

            scanf( "%lld", &first );
            stackPush( stk, first * PRECISION );

            ON_LOGGING(printf( "[%d] IN CALL, PUSHED %lld\n", step, first ));
        }

        else if ( command == COMMANDS::OUT ){

            stackPop( stk, &first );
            ON_LOGGING(printf( "---\n[OUT]: "));

            printf("%.3lf\n", first / PRECISION);

            ON_LOGGING(printf("\n---\n" ));
        }

        else if ( command == (COMMANDS::CALL | I_BIT) ){

            elem_t address = *(elem_t*) ( processor.cs + processor.ip );
            stackPush( recursionStack, processor.ip );
            processor.ip = address;

        }

        else if ( command == COMMANDS::RET ){
            elem_t address = 0;

            stackPop( recursionStack, &address );

            processor.ip = address + sizeof( uint64_t );
        }

        else if ( command == COMMANDS::HLT ){
            ON_LOGGING(printf( "[%d] HLT, breaking\n", step ));
            break;
        }

        else if ( command == COMMANDS::SP  ){ // data must be stored in such way x y r g b
            elem_t x = 0, y = 0;

            elem_t r = 0;
            elem_t g = 0;
            elem_t b = 0;

            stackPop( stk, &b );
            stackPop( stk, &g );
            stackPop( stk, &r );
            stackPop( stk, &y );
            stackPop( stk, &x );

            r /= PRECISION;
            g /= PRECISION;
            b /= PRECISION;
            x /= PRECISION;
            y /= PRECISION;


            if ( r < 0 || r > 255 ||
                 g < 0 || g > 255 ||
                 b < 0 || b > 255 ){
                    printf( "[%d] ERROR: incorrect pixel color (%d, %d, %d)\n", step, r, g, b );
                 }
            else if ( x < 0 || x > PROC_WINDOW_LENGTH ||
                      y < 0 || y > PROC_WINDOW_LENGTH ){
                    printf( "[%d] ERROR: incorrect pixel coordinates (%d, %d)\n", step, x, y );
                    }
            else{
                processor.GPU[y][x].r = r;
                processor.GPU[y][x].g = g;
                processor.GPU[y][x].b = b;
            }
        }

        else if ( command == COMMANDS::UW ){
            for ( int pos_y = 1; pos_y < PROC_WINDOW_HEIGHT; pos_y++ ){
                for (int pos_x = 0; pos_x < PROC_WINDOW_LENGTH; pos_x++ ){
                    drawRect ( dc, pos_x * COMPRESSION, pos_y * COMPRESSION,
                                           processor.GPU[pos_y][pos_x].r,
                                           processor.GPU[pos_y][pos_x].g,
                                           processor.GPU[pos_y][pos_x].b );
                }
            }
            txRedrawWindow();
            txSleep (1000 / 60.0);

        }

        else if ( command == COMMANDS::CW ){
            txCreateWindow(ACTUAL_WINDOW_LENGTH, ACTUAL_WINDOW_HEIGHT);
            txBegin();
        }

        else
        #include "jmps.h"
        {
            printf( "[%d] UNKNOWN COMMAND %d\n", step, command );
            break;
        }
        step++;
    }

    SPUDtor(&processor);
    return 0;
}


void push( stack* stk, elem_t* first, elem_t* second, elem_t ( *func ) ( elem_t, elem_t ) ){
    stackPop( stk, first );
    stackPop( stk, second );
    stackPush( stk, func( *first, *second ) );
}


elem_t _add_( elem_t first, elem_t second ){
    return first + second;
}

elem_t _sub_( elem_t first, elem_t second ){
    return second - first;
}

elem_t _mul_( elem_t first, elem_t second ){
    return first * second / PRECISION;
}

elem_t _div_( elem_t first, elem_t second ){

    if ( first == 0 )
        return second;

    double tmp = second / ( double ) first;
    return ( elem_t ) ( tmp * PRECISION );
}

void SPUCtor( SPU* spu ){
    stackCtor( &spu->stk );
    stackCtor( &spu->rs  );

    spu->cs  = calloc( sizeof( char ),     CS_BUFFER_SIZE );
    spu->ip  = 0;

    memset( spu->GPU, 0, sizeof( pixel ) * PROC_WINDOW_HEIGHT * PROC_WINDOW_LENGTH );

    spu->ram  = ( elem_t* ) calloc( sizeof( char ),     RAM_SIZE );
    spu->size = RAM_SIZE;
    spu->pos  = 0;

    spu->rax = 0;
    spu->rbx = 0;
    spu->rcx = 0;
    spu->rdx = 0;
}

void SPUDtor( SPU* spu ){
    stackDtor( &spu->stk );
    stackDtor( &spu->rs  );

    memset( spu->GPU, 0, sizeof( pixel ) * PROC_WINDOW_HEIGHT * PROC_WINDOW_LENGTH );

    free( spu->cs  );
    spu->ip   = -1;

    free( spu->ram );
    spu->size = -1;
    spu->pos  = -1;

    spu->rax  = -1;
    spu->rbx  = -1;
    spu->rcx  = -1;
    spu->rdx  = -1;
}

void drawRect(RGBQUAD* PIXELS, int x, int y, int r, int g, int b){
    static POINT scr = txGetExtent();
    RGBQUAD rgb = {r, g, b};
    for ( int pos_x = 0; pos_x < COMPRESSION; pos_x++ ){
    for ( int pos_y = 0; pos_y < COMPRESSION; pos_y++ ){
        *(txVideoMemory() + x + pos_x + (-y - pos_y + scr.y) * scr.x) = rgb;

    }
    }
}
