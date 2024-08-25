# Little Compiler
This is mine stack-working assembler, that is executed inside of JIT compiler.
## Configure
Configure the project via:
```
make
```
__jit__ and __assembly__ executable files should appear.

## Usage
Create file and write assembly code. After you done coding, compile it via
```
./assembly <FILE_NAME>
```
__result.bin__ file should appear.
Run your binary code using:
```
./jit <FILE_NAME>
```

## Conventions
Any LABEL name should start with ":":
```
call SomeLabel_Name // calling label

:SomeLabel_Name // the label
```
Be aware, that JCC commands modify R10, R11 [volitiledable registers](https://en.wikipedia.org/wiki/X86_calling_conventions). Additionally, the assembly language program you write must follow [the conventions](registers)(Windows and Linux platforms should work).

## Commands
List of supported operations and how are they appear in jit buffer:
### ```PUSH IMM/REG```
Pushes immediate/register value into the stack.
Example, the result will be top of the stack containing 100:
```
push 100
```

### ```POP REG```
Pops value from the top of the stack into referenced register.
Example, the result will be RAX containing 100:
```
push 100
pop rax
```


### ```ADD```
Pops the first 2 values from the top of the stack and pushes their sum, pushing result to stack. Does not modify any registers.
Example, the result will be RAX containing 120:
```
push 100
push 20
add
pop rax
```
### ```SUB```
Pops the first 2 values ​​from the top of the stack and subtracts the first value from the second, pushing result to stack. Does not modify any register.
Example, the result will be RAX containing 100:
```
push 120
push 20
sub
pop rax
```
### ```MUL```
Pops the first 2 values from the top of the stack and multiplies them, pushing result to stack. Does not modify any register.
Example, the result will be RAX containing 120:
```
push 6
push 20
mul

pop rax
```
### ```DIV```
Assumes, that the dividend is located in RAX register. Pops value from the top of the stack and divides with it RAX register. Quotient stays in the RAX register, remainder - in RDX.
Example, the result will be RAX containing 20, RDX - 0:
```
push 120
pop rax

push 6
div
```
### ```JMP <LABEL>```
Moves to the LABEL location and continues executing code. __EVERY LABEL SHOULD START WITH ":"__. Does not modify any register.
Example, the result will be RAX containing 100:
```
push 100
jmp SOME_LABEL
pop rbx

:SOME_LABLE
pop rax
```
### ```JCC <LABEL>```
Pops 2 values from the top of the stacl, jumps to the LABEL if the condition is met
| Command   |   Condition |
| --------- | ----------- |
|JA  | second_value >  first_value|
|JAE | second_value >= first_value|
|JB  | second_value <  first_value|
|JBE | second_value <= first_value|
|JE  | second_value == first_value|
|JNE | second_value != first_value|

Example of JB, the result will be RAX containing 20.
Note: if we replace JB with JA, RAX will contain 100:
```
push 10     // second value
push 2      // first value
jbe Some_Label
push 20
jmp Jump_Here

:Some_Label
push 100


:Jump_Here
pop rax
```

### ```CALL <LABEL>```
Pushes RIP value to the stack move to the LABEL, serves as function calling. To go back to the CALL location, use RET.
Example: compile __factorial.txt__ file.

### ```RET```
Pops value from the top of the stack and jumps to it. Combine with CALL.

## Used technologies
Assembly code converts into __binary format__, then read by jit.exe and placed into __IR (intermediate representation)__, which is used when loading the __jit buffer__, which is eventually executed.

IR has following structure

__assembler command - command from the file you wrote__
__assembler Instruction - command from actual assembler__
__All empty fields must be DEAD_VALUE.__
```
IR_Array
|* size                       // number of written IR_Items
|* capacity                   // number of maximum IR_Items
|* IR_Items[]                 // Struct, that represents ONE assembler command
\
 \
  \
   |* total_bytes            // total bytes that is needed to represent assembler command. This value should always be equals to the sum of all total_bytes of IR_Intrustions
   |* pos_in_file            // position in file you wrote
   |* actual_address         // counting all space previous IR_Items can take. Basiclly, equals to the sum of IR_Items.total_bytes of all previous IR_Items
   |* IR_Instructions[20]    // so 20 assembler Instructions can represent one THIS assembler command
    \
     \
      \
       |* total_bytes        // total bytes the instruction use
       |* is_jmp_instruction // needed to locate labels properly
       |
       | // following fields encodes the assembler Instruction
       |* prefixes[4]
       |* op_code[3]
       |* modR
       |* SIB
       |* displacement[4]
       |* immediate[4]
```
