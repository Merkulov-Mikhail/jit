COMPILER=g++ -O0 -g -Wno-pointer-arith


.PHONY: all

all: assembly jit


assembly: 	build/assembly.o
	$(COMPILER) $< -o $@

jit:		build/jit.o src/ir.h src/x86.h
	$(COMPILER) $< -o $@

build/%.o:	 src/%.c src/config.h
	$(COMPILER) -c $< -o $@
