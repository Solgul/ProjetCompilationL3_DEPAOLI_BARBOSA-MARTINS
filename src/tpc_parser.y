%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Pour l'option -t
#include "tree.h"

extern int yylex(void);
extern int yylineno;

void yyerror(const char *s);

int exit_status = 0;
int print_tree = 0;
Node *root = NULL;
%}

%locations
%expect 1

%union {
    char* str;
    int num;
    Node* node;
}

%token <str> IDENT
%token <num> NUM CHARACTER TYPE EQ ORDER ADDSUB DIVSTAR
%token VOID IF ELSE WHILE RETURN STRUCT
%token OR AND

%type <node> Prog DeclVars DeclFoncts DeclFonct EnTeteFonct Parametres
%type <node> ListTypVar Corps SuiteInstr Instr Exp TB FB M E T F
%type <node> Arguments ListExp Declarateurs StructDecl StructFieldList StructField
%type <node> VarType LValue

%start Prog

%%

Prog: DeclVars DeclFoncts
    {
        root = makeNode(E);
        if ($1) addChild(root, $1);
        if ($2) addChild(root, $2);
        $$ = root;
    }
    ;

DeclVars:
    DeclVars VarType Declarateurs ';'
    {
        $$ = $1;
        if ($$ == NULL) $$ = makeNode(T);
        Node* decl = makeNode(T);
        addChild(decl, $2);
        addChild(decl, $3);
        addChild($$, decl);
    }
    | DeclVars StructDecl
    {
        $$ = $1;
        addChild($$, $2);
    }
    | /* empty */
    {
        $$ = makeNode(T);
    }
    ;

VarType:
    TYPE
    {
        $$ = makeNode(ID_NODE);
    }
    | STRUCT IDENT
    {
        $$ = makeNode(ID_NODE);
        addChild($$, makeNode(ID_NODE));
    }
    ;

StructDecl:
    STRUCT IDENT '{' StructFieldList '}' ';'
    {
        $$ = makeNode(T);
        addChild($$, makeNode(ID_NODE));
        addChild($$, $4);
    }
    ;

StructFieldList:
    StructFieldList StructField
    {
        $$ = $1;
        addChild($$, $2);
    }
    | StructField
    {
        $$ = makeNode(E);
        addChild($$, $1);
    }
    ;

StructField:
    VarType Declarateurs ';'
    {
        $$ = makeNode(T);
        addChild($$, $1);
        addChild($$, $2);
    }
    ;

Declarateurs:
    Declarateurs ',' IDENT
    {
        $$ = $1;
        addChild($$, makeNode(ID_NODE));
    }
    | IDENT
    {
        $$ = makeNode(E);
        addChild($$, makeNode(ID_NODE));
    }
    ;

DeclFoncts:
    DeclFoncts DeclFonct
    {
        $$ = $1;
        addChild($$, $2);
    }
    | DeclFonct
    {
        $$ = makeNode(E);
        addChild($$, $1);
    }
    ;

DeclFonct: EnTeteFonct Corps
    {
        $$ = makeNode(E);
        addChild($$, $1);
        addChild($$, $2);
    }
    ;

EnTeteFonct:
    VarType IDENT '(' Parametres ')'
    {
        $$ = makeNode(T);
        addChild($$, $1);
        addChild($$, makeNode(ID_NODE));
        addChild($$, $4);
    }
    | VOID IDENT '(' Parametres ')'
    {
        $$ = makeNode(T);
        addChild($$, makeNode(DIVSTAR_NODE)); // void
        addChild($$, makeNode(ID_NODE));
        addChild($$, $4);
    }
    ;

Parametres:
    VOID { $$ = makeNode(T); }
    | ListTypVar { $$ = $1; }
    ;

ListTypVar:
    ListTypVar ',' VarType IDENT
    {
        $$ = $1;
        Node* p = makeNode(T);
        addChild(p, $3);
        addChild(p, makeNode(ID_NODE));
        addChild($$, p);
    }
    | VarType IDENT
    {
        $$ = makeNode(T);
        Node* p = makeNode(T);
        addChild(p, $1);
        addChild(p, makeNode(ID_NODE));
        addChild($$, p);
    }
    ;

Corps: '{' DeclVars SuiteInstr '}'
    {
        $$ = makeNode(T);
        addChild($$, $2);
        addChild($$, $3);
    }
    ;

SuiteInstr:
    SuiteInstr Instr
    {
        $$ = $1;
        if ($2) addChild($$, $2);
    }
    | /* empty */
    {
        $$ = makeNode(E);
    }
    ;

LValue:
    IDENT
    {
        $$ = makeNode(ID_NODE);
    }
    | LValue '.' IDENT
    {
        $$ = makeNode(T);
        addChild($$, $1);
        addChild($$, makeNode(ID_NODE));
    }
    ;

Instr:
    LValue '=' Exp ';'
    {
        $$ = makeNode(E);
        addChild($$, $1);
        addChild($$, $3);
    }
    | IF '(' Exp ')' Instr
    {
        $$ = makeNode(E);
        addChild($$, $3);
        addChild($$, $5);
    }
    | IF '(' Exp ')' Instr ELSE Instr
    {
        $$ = makeNode(T);
        addChild($$, $3);
        addChild($$, $5);
        addChild($$, $7);
    }
    | WHILE '(' Exp ')' Instr
    {
        $$ = makeNode(DIVSTAR_NODE);
        addChild($$, $3);
        addChild($$, $5);
    }
    | IDENT '(' Arguments ')' ';'
    {
        $$ = makeNode(T);
        addChild($$, makeNode(ID_NODE));
        addChild($$, $3);
    }
    | RETURN Exp ';'
    {
        $$ = makeNode(E);
        addChild($$, $2);
    }
    | RETURN ';'
    {
        $$ = makeNode(E);
    }
    | '{' SuiteInstr '}'
    {
        $$ = $2;
    }
    | ';' { $$ = NULL; }
    ;

Exp: Exp OR TB
    {
        $$ = makeNode(OR_BOOL);
        addChild($$, $1);
        addChild($$, $3);
    }
    | TB { $$ = $1; }
    ;

TB: TB AND FB
    {
        $$ = makeNode(AND_BOOL);
        addChild($$, $1);
        addChild($$, $3);
    }
    | FB { $$ = $1; }
    ;

FB: FB EQ M
    {
        label_t label = ($2 == 0) ? EQ_EQUAL : EQ_NEQ;
        $$ = makeNode(label);
        addChild($$, $1);
        addChild($$, $3);
    }
    | M { $$ = $1; }
    ;

M: M ORDER E
    {
        label_t label;
        switch($2) {
            case 0: label = LT; break;
            case 1: label = LEQ; break;
            case 2: label = GT; break;
            case 3: label = GEQ; break;
            default: label = ID_NODE; break;
        }
        $$ = makeNode(label);
        addChild($$, $1);
        addChild($$, $3);
    }
    | E { $$ = $1; }
    ;

E: E ADDSUB T
    {
        label_t label = ($2 == 0) ? ADD : SUB;
        $$ = makeNode(label);
        addChild($$, $1);
        addChild($$, $3);
    }
    | T { $$ = $1; }
    ;

T: T DIVSTAR F
    {
        /* Distinction *, /, % */
        label_t label;
        switch($2) {
            case 0: label = MULT; break;
            case 1: label = DIV; break;
            case 2: label = MOD; break;
            default: label = DIVSTAR_NODE; break; 
        }
        $$ = makeNode(label);
        addChild($$, $1);
        addChild($$, $3);
    }
    | F { $$ = $1; }
    ;

F: ADDSUB F
    {
        label_t label = ($1 == 0) ? ADD : SUB;
        $$ = makeNode(label);
        addChild($$, $2);
    }
    | '!' F
    {
        $$ = makeNode(NOT);
        addChild($$, $2);
    }
    | '(' Exp ')' { $$ = $2; }
    | NUM { $$ = makeNode(ID_NODE); }
    | CHARACTER { $$ = makeNode(ID_NODE); }
    | IDENT '(' Arguments ')'
    {
        $$ = makeNode(T);
        addChild($$, makeNode(ID_NODE));
        addChild($$, $3);
    }
    | LValue
    {
        $$ = $1;
    }
    ;

Arguments:
    ListExp { $$ = $1; }
    | /* empty */
    {
        $$ = makeNode(E);
    }
    ;

ListExp:
    ListExp ',' Exp
    {
        $$ = $1;
        addChild($$, $3);
    }
    | Exp
    {
        $$ = makeNode(E);
        addChild($$, $1);
    }
    ;

%%

void print_help(const char *progname) {
    printf("Usage: %s [options] < [path]\n", progname);
    printf("Options:\n");
    printf("  -t        Affiche l'arbre syntaxique (AST)\n");
    printf("  -h        Affiche cette aide\n");
}

int main(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "th")) != -1) {
        switch (opt) {
            case 't':
                print_tree = 1;
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Usage: %s [-t] [file]\n", argv[0]);
                return 1;
        }
    }
    yyparse();
    if (exit_status == 0 && root && print_tree) {
        printTree(root);
        deleteTree(root);
    }
    else{
        printf("%d\n", exit_status);
    }
    return exit_status;
}

void yyerror(const char *s) {
    exit_status = 1;
    fprintf(stderr, "Erreur syntaxique ligne %d, colonne %d : %s\n",
            yylloc.first_line, yylloc.first_column, s);
}
