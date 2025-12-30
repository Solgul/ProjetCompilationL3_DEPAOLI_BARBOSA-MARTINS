CC = gcc
CFLAGS = -Wall -g -Iobj -Isrc

YACC = bison
LEX = flex

PARSER = parser
LEXER = lexer

BINARY = bin
SOURCE = src
TEST = test
OBJ = obj
EXEC = $(BINARY)/tpcas

all : $(EXEC)

$(EXEC) : $(OBJ)/$(LEXER).o $(OBJ)/$(PARSER).o $(OBJ)/tree.o
	@mkdir -p $(BINARY)
	$(CC) -o $@ $^

$(OBJ)/tree.o : $(SOURCE)/tree.c $(SOURCE)/tree.h

$(OBJ)/%.o : $(SOURCE)/%.c
	@mkdir -p $(OBJ)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(PARSER).c $(OBJ)/$(PARSER).h &: $(SOURCE)/$(PARSER).y 
	$(YACC) -d $< -o $@

$(OBJ)/$(LEXER).c : $(SOURCE)/$(LEXER).lex $(OBJ)/$(PARSER).h
	$(LEX) -o $@ $<


clean:
	rm -f obj/*