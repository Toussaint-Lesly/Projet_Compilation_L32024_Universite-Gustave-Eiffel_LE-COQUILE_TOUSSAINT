# Author : LE COQUIL - TOUSSAINT

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite

include ./makefiles/makefile_const

ASM_SRCS=$(wildcard *.asm)
ASM_OBJS=$(ASM_SRCS:.asm=.o)
ASM_EXECS=$(ASM_SRCS:.asm=)

OBJS = $(TREE_OBJS) $(COMP_OBJS) ./$(OBJ)/main.o

all: $(EXEC)

$(EXEC): mrproper create_obj_and_bin_folders_if_necessary $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@
	mv $(EXEC) ./$(BIN)

create_obj_and_bin_folders_if_necessary:
	mkdir -p ./$(BIN)
	mkdir -p ./$(OBJ)

$(OBJ)/main.o: ./$(SRC)/main.c ./$(OBJ)/tpcas.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TREE_OBJS):
	make -f ./makefiles/maketree

$(COMP_OBJS):
	make -f ./makefiles/makecomp

assemble: $(ASM_EXECS)

%: %.o
	gcc -g -o $@ $< -no-pie -nostartfiles
	rm -f $<

%.o: %.asm
	nasm -f elf64 -o $@ $<

clean:
	rm -f ./obj/*

uninstall:
	rm -f ./bin/*

mrproper: clean uninstall
