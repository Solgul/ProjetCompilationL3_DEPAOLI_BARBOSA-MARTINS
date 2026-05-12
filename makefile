CC = gcc
CFLAGS = -Wall -g -Iobj -Isrc
LDFLAGS = -nostartfiles -no-pie

YACC = bison
LEX = flex

EXEC_ASM = _anonymous
NASM = nasm
ASFLAGS = -f elf64 -g

PARSER = parser
LEXER = lexer

BINARY = bin
SOURCE = src
TEST = test
OBJ = obj
ASM = asm
EXEC = $(BINARY)/tpcc

all : clean create $(EXEC)

create : 
	@mkdir -p $(BINARY)
	@mkdir -p $(OBJ)
	@mkdir -p $(ASM)

$(EXEC) : $(OBJ)/main.o $(OBJ)/$(LEXER).o $(OBJ)/$(PARSER).o $(OBJ)/tree.o $(OBJ)/asm.o $(OBJ)/sym.o 
	$(CC) -o $@ $^

$(OBJ)/main.o : $(SOURCE)/main.c $(SOURCE)/tree.h $(SOURCE)/sym.h $(SOURCE)/asm.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/tree.o : $(SOURCE)/tree.c $(SOURCE)/tree.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/asm.o: $(SOURCE)/asm.c $(SOURCE)/asm.h $(SOURCE)/tree.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/sym.o : $(SOURCE)/sym.c $(SOURCE)/sym.h $(SOURCE)/tree.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(PARSER).o: $(OBJ)/$(PARSER).c $(SOURCE)/tree.h $(SOURCE)/sym.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(LEXER).o: $(OBJ)/$(LEXER).c $(OBJ)/$(PARSER).h 
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(LEXER).c : $(SOURCE)/$(LEXER).lex $(OBJ)/$(PARSER).h
	$(LEX) -o $@ $<

$(OBJ)/$(PARSER).c $(OBJ)/$(PARSER).h &: $(SOURCE)/$(PARSER).y
	$(YACC) -d -o $(OBJ)/$(PARSER).c $<

asm : $(ASM)/$(EXEC_ASM).o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o : %.asm
	$(NASM) $(ASFLAGS) -o $@ $<

clean:
	rm -fr $(OBJ)
	rm -fr $(BINARY)
	rm -fr $(ASM)

test: all
	chmod +x script.sh
	./script.sh || true
