#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "tree.h"

int yyparse() { return 0; }
FILE *yyin;
int yylineno;
Node *root = NULL;


void print_help(const char *exeName);
int handle_opt(int argc, char *argv[]);