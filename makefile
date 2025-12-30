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

$(BINARY)/tpcas.o : $(OBJ)/$(PARSER).o $(OBJ)/$(PARSER).o $(OBJ)/$(PARSER).o 

$(OBJ)/tree.o : $(SOURCE)/tree.c $(SOURCE)/tree.h

$(OBJ)/%.o : $(SOURCE)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(LEXER).c : $(SOURCE)/$(LEXER).lex
	$(LEX) -o $@ $<

$(OBJ)/$(PARSER).c $(OBJ)/$(PARSER).h &: $(SOURCE)/$(PARSER).y 
	$(YACC) -d $< -o $@

clean:
	rm -f obj/*