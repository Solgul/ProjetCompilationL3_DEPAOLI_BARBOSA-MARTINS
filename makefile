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

$(BINARY)/tpcas.o : $(OBJ)/$(LEXER).o $(OBJ)/$(PARSER).o $(OBJ)/tree.o
	$(CC) -o $@ $^

$(OBJ)/tree.o : $(SOURCE)/tree.c $(SOURCE)/tree.h

$(OBJ)/%.o : $(SOURCE)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(PARSER).c $(OBJ)/$(PARSER).h &: $(SOURCE)/$(PARSER).y 
	$(YACC) -d $< -o $@

$(OBJ)/$(LEXER).c : $(SOURCE)/$(LEXER).lex $(OBJ)/$(PARSER).h
	$(LEX) -o $@ $<


clean:
	rm -f obj/*