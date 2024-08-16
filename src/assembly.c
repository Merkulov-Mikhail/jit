#include "config.h"

#include <string.h>
#include <stdio.h>
#include <math.h>


#define ADD_COMMAND( name, num, nargs, ... )\
    if ( !strcmp( command, #name ) ){\
        comName  = #name;\
        comNum   = num;\
        comNargs = nargs;\
    }\
    else\



void destroyCommentary (   char* arr );
int  parseArgument ( const char* buf );
int isRegister     ( const char* buf );


struct LabelPoints{
    uint64_t     pc;
    char* labelName;
};


int main(int argc, char* argv[]){
    FILE* input = nullptr;
    if ( argc == 2 )
        input = fopen( argv[1], "r" );
    else
        input = fopen( INPUT_FILE, "r" );
    FILE* output = fopen( ASSEMBLY_FILE, "wb" );

    if ( !input ){
        printf( "Error: no input file" );
        return -1;
    }
    if ( !output ){
        printf( "Error: could not create final file" );
        return -1;
    }

    int size = CS_BUFFER_SIZE;
    void* commandSegment = calloc( sizeof( char ), size );

    // Shows how many bytes have been written in current page
    uint64_t usedBytes       = 16;

    // Shows how many bytes have been written in total
    // Used to write named labels
    uint64_t globalUsedBytes = 0;

    uint64_t step            = 1;
    int retValue             = 0;

    char command[MAX_LINE_LENGTH] = {};

    Label* labels =            ( Label* )       calloc( sizeof( Label ), LABEL_COUNT );
    uint64_t labelCounter = 0;

    LabelPoints* labelPoints = ( LabelPoints* ) calloc( sizeof( labelPoints ), LABEL_COUNT );
    uint64_t labelPointsCounter = 0;

    char argumentBuffer[MAX_LINE_LENGTH] = {};

    while ( 1 ){

        retValue = fscanf( input, "%s", command );

        const char* comName = nullptr;
        char comNum = -1, comNargs = 0;

        if ( retValue == 0 ){
            printf( "[%d] ERROR\n", step );
            break;
        }
        else if ( retValue == -1 ){
            printf( "[%d] EOF\n",   step );
            break;
        }

        if ( command[0] == ':' ){

            labels[labelCounter].pi   = globalUsedBytes;

            labels[labelCounter].labelName = ( char* ) calloc( sizeof( char ), strlen( command ) );
            strncpy( labels[labelCounter].labelName, command + 1, strlen( command ) );

            labelCounter++;

            continue;
        }
        #include "commands.h"
        /*else*/{
            printf( "[%d] UNKNOWN COMMAND %s\n", step, command );
            break;
        }
        if ( !comNargs ){
            *( char* ) ( commandSegment + usedBytes++ ) = comNum;
            globalUsedBytes++;
        }
        else {

            retValue = fscanf( input, "%[^\n]", argumentBuffer );

            char* buf = argumentBuffer;

            if ( retValue <= 0 ){
                printf( "[%d] ERROR: %s with no argument\n", step, comName );
                break;
            }

            destroyCommentary( buf );
            while ( *buf == ' ' ){
                buf++;
            }

            int doBreak = 0;
            if ( ( JMP <= comNum && comNum <= JE ) || comNum == CALL  ) {

                double value = 0;
                int res = sscanf( buf, "%lf %s", &value );
                *( char* ) ( commandSegment + usedBytes++ ) = comNum | I_BIT;
                globalUsedBytes++;

                if ( res == 0 ){ // label is not an clean address, it's a name

                    char labelName[MAX_LINE_LENGTH] = {};
                    res = sscanf( buf, "%s %s", labelName );

                    switch ( res ){ // qweasd found
                        case 2:
                            printf( "[%d] ERROR: Unknown data after argument\n", step );
                            doBreak = 1;
                            break;
                        case 0:
                            printf( "[%d] ERROR: No argument or line limit exceeded\n", step );
                            doBreak = 1;
                            break;
                        case 1:

                            labelPoints[labelPointsCounter].pc = globalUsedBytes;
                            labelPoints[labelPointsCounter].labelName = ( char* ) calloc( sizeof( char ), strlen( labelName ) );

                            strncpy( labelPoints[labelPointsCounter].labelName, labelName, strlen( labelName ) );

                            labelPointsCounter++;
                            usedBytes += sizeof( double );
                            globalUsedBytes += sizeof(double);

                    }
                }
                else if ( res == 1 ){
                    *( double* ) ( commandSegment + usedBytes ) = value;
                    usedBytes += sizeof( double );
                    globalUsedBytes += sizeof( double );
                }
                else{
                    printf( "[%d] ERROR: Unknown data after argument\n", step );
                    doBreak = 1;
                }
            }
            else{


                int res = parseArgument( buf );
                /*
                    See ARGUMENT_RESPONSE
                    If res is not present in ARGUMENT_RESPONSE, than res contains register address if returned value is positive, othervise -
                */
                switch ( res ){

                    case LINE_ERROR:
                        printf( "[%d] ERROR: No argument or line limit exceeded", step );
                        doBreak = 1;
                        break;

                    case BAD_ARGUMENT:
                        printf( "[%d] ERROR: Incorrect argument", step );
                        doBreak = 1;
                        break;

                    case UNKNOWN_DATA:
                        printf( "[%d] ERROR: Unknown data after argument", step );
                        doBreak = 1;
                        break;

                    case IS_NUMBER:
                        *( char* )   ( commandSegment + usedBytes++ ) = comNum | I_BIT;
                        globalUsedBytes++;

                        sscanf( buf, "%lf", ( double* ) ( commandSegment + usedBytes ) );

                        usedBytes += sizeof( double );
                        globalUsedBytes += 8;

                        break;

                    case IS_RAM_NUMBER:
                        *( char* )   ( commandSegment + usedBytes++ ) = comNum | I_BIT | M_BIT;
                        globalUsedBytes++;

                        sscanf( buf, "[%lld]", ( int64_t* ) ( commandSegment + usedBytes ) );

                        usedBytes += sizeof( int64_t );
                        globalUsedBytes += 8;

                        break;

                    default:

                        char checker[5] = {};
                        sscanf( buf, "%s", checker );

                        if ( checker[0] == '[' && checker[4] == ']' )
                            *( char* ) ( commandSegment + usedBytes++ ) = comNum | R_BIT | M_BIT;
                        else
                            *( char* ) ( commandSegment + usedBytes++ ) = comNum | R_BIT;

                        *( char* ) ( commandSegment + usedBytes++ ) = res;

                        globalUsedBytes += 2;
                        break;

                }
            }
            if ( doBreak )
                break;
        }
        #ifdef LOGGING
            if ( step % 100000 == 0 )
                printf( "[%d] %s COMMAND\n", step, comName );
        #endif
        step++;
        if ( usedBytes + 12 >= size ){

            fwrite( commandSegment, sizeof( char ), usedBytes, output );

            usedBytes = 0;
        }

    }
    fclose( input );

    fwrite( commandSegment, sizeof( char ), usedBytes, output );
    for (     uint64_t pointsPos = 0; pointsPos < labelPointsCounter; pointsPos++ ) {
        int foundLabel = 0;

        for ( uint64_t labelsPos = 0; labelsPos < labelCounter;       labelsPos++ ) {

            if ( !strcmp( labels[labelsPos].labelName, labelPoints[pointsPos].labelName ) ){

                fseek( output, labelPoints[pointsPos].pc, SEEK_SET );
                fwrite( &( labels[labelsPos].pi ), sizeof( uint64_t ), 1, output );

                foundLabel = 1;
                break;
            }
        }
        if ( !foundLabel ){
            printf( "ERROR: unknown label ( %s )\n", labelPoints[pointsPos].labelName );
            break;
        }
    }
	
    fseek(output, 0, SEEK_SET);
    fwrite( OWNER_NAME, sizeof( char ), 8, output );
    fwrite( &( globalUsedBytes ), sizeof( uint64_t ), 1, output );

    fclose( output );

    //---------------------------------------------------
    for ( uint64_t pos = 0; pos < labelPointsCounter; pos++ ){
        free( labelPoints[pos].labelName );
    }

    free( labelPoints );

    for ( uint64_t pos = 0; pos < labelCounter; pos++ ){
        free( labels[pos].labelName );
    }

    free( labels );
    free( commandSegment );
    //---------------------------------------------------
}


void destroyCommentary( char* arr ){
    char* pos = strchr( arr, ';' );
    if ( pos )
        arr[pos - arr] = '\0';
}

int parseArgument( const char* buf ){
    double elem = 0;

    int retValue = sscanf( buf, "%lf", &elem ); // if argument is a immediate const

    if ( retValue ){

        if ( sscanf( buf, "%*s %*s" ) == 2 ) // catching qweasd
            return UNKNOWN_DATA;
        else
            return IS_NUMBER;
    }

    else{
        int res = isRegister(buf);
        if ( res != BAD_ARGUMENT || res > -1 )
            return res;

        // there is a possibility of address as constant ([1234])
        buf++;
        if ( sscanf( buf, "%lf", &elem ) == 1 ){
            return IS_RAM_NUMBER;
        }
        else{
            return LINE_ERROR;
        }
    }
}

int isRegister(const char* buf){
    if ( buf[0] == '[' && buf[4] == ']' )
        buf++;

    if ( buf[0] == 'r' && buf[2] == 'x' && buf[1] - 'a' < 4 ){ // push r?x qweasd

        if ( sscanf( buf, "%*s %*s" ) == 2 ) // catching qweasd
            return UNKNOWN_DATA;

        else // no qweasd, all Good
            return buf[1] - 'a';
    }
    else{ // not a valiable register
        return BAD_ARGUMENT;
    }
}
