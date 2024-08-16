#include "config.h"

#include <string.h>
#include <stdio.h>
#include <math.h>


int main(){
    FILE* input  = fopen( ASSEMBLY_FILE,    "r" );
    FILE* output = fopen( DISASSEMBLY_FILE, "w" );

    if ( !input ){
        printf( "Error: no input file" );
        return -1;
    }
    if ( !output ){
        printf( "Error: could not create final file" );
        return -1;
    }

    int retValue  = 0;
    int command = 0;
    int step = 1;

    while ( 1 ){
        retValue = fscanf( input, "%d", &command );
        if ( retValue <= 0 ){
            printf( "ERROR/EOF\n" );
            break;
        }

        if      ( command == COMMANDS::ADD ){

            fprintf( output, "add\n" );
            printf( "[%d] ADD COMMAND\n", step );
        }

        else if ( command == COMMANDS::SUB ){

            fprintf( output, "sub\n" );
            printf( "[%d] SUB COMMAND\n", step );
        }

        else if ( command == COMMANDS::MUL ){

            fprintf( output, "mul\n" );
            printf( "[%d] MUL COMMAND\n", step );
        }

        else if ( command == COMMANDS::DIV ){

            fprintf( output, "div\n" );
            printf( "[%d] DIV COMMAND\n", step );
        }

        else if ( command == COMMANDS::PUSH ){

            double elem = 0;
            retValue = fscanf( input, "%lf", &elem );

            if ( retValue <= 0 ){
                printf( "ERROR\n" );
                break;
            }

            fprintf( output, "push %lf\n", elem );
            printf( "[%d] PUSH COMMAND\n", step );
        }
        else if ( command == COMMANDS::SQRT ){

            fprintf( output, "sqrt\n" );
            printf( "[%d] SQRT COMMAND\n", step );
        }
        else if ( command == COMMANDS::SIN ){

            fprintf( output, "sin\n" );
            printf( "[%d] SIN COMMAND\n", step );
        }
        else if ( command == COMMANDS::COS ){

            fprintf( output, "cos\n" );
            printf( "[%d] COS COMMAND\n", step );
        }
        else if ( command == COMMANDS::IN ){

            fprintf( output, "in\n" );
            printf( "[%d] IN COMMAND\n", step );
        }
        else if ( command == COMMANDS::OUT ){

            fprintf( output, "out\n" );
            printf( "[%d] OUT COMMAND\n", step );
        }
        else if ( command == COMMANDS::HLT ){

            fprintf( output, "hlt\n" );
            printf( "[%d] HLT COMMAND\n", step );
        }
        else {

            printf( "[%d] UNKNOWN COMMAND %d\n", step, command );
            break;
        }
        step++;
    }
    fclose( input );
    fclose( output );
}
