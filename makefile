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

all : clean $(EXEC)

$(EXEC) : $(OBJ)/$(LEXER).o $(OBJ)/$(PARSER).o $(OBJ)/tree.o $(OBJ)/sym.o 
	@mkdir -p $(BINARY)
	$(CC) -o $@ $^

$(OBJ)/tree.o : $(SOURCE)/tree.c $(SOURCE)/tree.h
	@mkdir -p $(OBJ)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/sym.o : $(SOURCE)/sym.c $(SOURCE)/sym.h $(SOURCE)/tree.h
	@mkdir -p $(OBJ)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(PARSER).o: $(OBJ)/$(PARSER).c $(SOURCE)/tree.h $(SOURCE)/sym.h
	@mkdir -p $(OBJ)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(LEXER).o: $(OBJ)/$(LEXER).c $(OBJ)/$(PARSER).h 
	@mkdir -p $(OBJ)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/$(LEXER).c : $(SOURCE)/$(LEXER).lex $(OBJ)/$(PARSER).h
	@mkdir -p $(OBJ)
	$(LEX) -o $@ $<

$(OBJ)/$(PARSER).c $(OBJ)/$(PARSER).h &: $(SOURCE)/$(PARSER).y
	@mkdir -p $(OBJ)
	$(YACC) -d -o $(OBJ)/$(PARSER).c $<

clean:
	rm -fr $(OBJ)
	rm -fr $(BINARY)