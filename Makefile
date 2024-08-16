COMPILER=g++ -O0 -g -Wno-pointer-arith


.PHONY: all

all: assembly jit


assembly: 	build/assembly.o
	$(COMPILER) $< -o $@

jit:		build/jit.o
	$(COMPILER) $< -o $@


build/%.o:	 src/%.c src/config.h
	$(COMPILER) -c $< -o $@
