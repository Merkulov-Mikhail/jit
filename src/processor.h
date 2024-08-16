#ifndef SPU_DEFINED
    #define SPU_DEFINED

    #include <inttypes.h>
    #include "stack.h"
    #include "config.h"

    struct pixel{
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

    struct SPU{
        stack stk;
        stack rs; // recursion stack

        int64_t rax;
        int64_t rbx;
        int64_t rcx;
        int64_t rdx;

        Label*   labels;

        void* cs;
        uint64_t ip;

        elem_t* ram ;
        int64_t pos ;
        int64_t size;

        pixel GPU[PROC_WINDOW_HEIGHT][PROC_WINDOW_LENGTH];
    };

#endif
