#ifndef CONFIG_DEFINED

    #define CONFIG_DEFINED


    #include <stdlib.h>
    #include <inttypes.h>


    #define LOGGING

    #undef LOGGING


    const char* INPUT_FILE       = "newfile.txt";
    const char* ASSEMBLY_FILE    = "result.bin";
    const char* OWNER_NAME	     = "NumMeRiL";

    const int VERSION         = 1;

    const short I_BIT         = 1 << 5;
    const short R_BIT         = 1 << 6;
    const short M_BIT         = 1 << 7;

    const int ASM_HEADER_SIZE = 16;
    const int MAX_LINE_LENGTH = 200;
    const uint64_t LABEL_COUNT = ( uint64_t ) 1024 * 1024 * 1024;

    const int64_t RAM_SIZE = 1024 * 1024 * sizeof( int64_t );

    const int RECURSION_LIMIT = 1000;

    const uint64_t CS_BUFFER_SIZE = ( uint64_t ) 1024 * 1024 * 1024;

    const int ACTUAL_WINDOW_LENGTH = 768;
    const int ACTUAL_WINDOW_HEIGHT = 576;

    const int COMPRESSION = 4;

    const int PROC_WINDOW_LENGTH = ACTUAL_WINDOW_LENGTH / COMPRESSION;
    const int PROC_WINDOW_HEIGHT = ACTUAL_WINDOW_HEIGHT / COMPRESSION;


    #undef IN
    #undef OUT


    enum COMMANDS{
        HLT = 0,

        PUSH,
        POP ,
        IN  ,
        OUT ,
        ADD ,
        SUB ,
        MUL ,
        DIV ,
        SQRT,
        SIN ,
        COS ,

        // Make sure, that JMP is THE FIRST command of jumps and JE is THE LAST command of jumps
        // Any other jmp command should be between JMP and JE
        JMP ,

        JA  , // >
        JAE , // >=
        JB  , // <
        JBE , // <=
        JNE , // !=
        JE  , // ==

        SP  , // gets 5 numbers from stack - b, g, r, y, x
        UW  , // updates window
        CW  , // creates window

        CALL,
        RET ,
    };


    enum ARGUMENT_RESPONSE{
        UNKNOWN_DATA  = -1,
        BAD_ARGUMENT  = -2,
        LINE_ERROR    = -3,
        IS_NUMBER     = -4,
        IS_RAM_NUMBER = -5,
    };


    struct Label{
        uint64_t      pi;
        char*  labelName;
    };

#endif
