%{
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

extern int yylex();
void yyerror(const char *s);
extern int yylineno;
Node *root;
%}

%type <node> DeclStruct DeclStart Prog Type DeclVars DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr IdentAffect Instr Exp TB FB M E T F Arguments ListExp

%%
Prog:  DeclStart DeclFoncts {
    $$ = makeNode(Prog);
    addChild($$, $1);
    addChild($$, $2);
    printTree($$);
    deleteTree($$);
}
    ;
DeclStruct : STRUCT ident '{' DeclVars '}'';'
    ;
DeclStart : DeclVars DeclStart
    | DeclStruct DeclStart
    |
    ; 
Type : TYPE
    | STRUCT IDENT
    ;
DeclVars:
       DeclVars Type Declarateurs ';'
    ;
Declarateurs:
       Declarateurs ',' IDENT
    |  IDENT
    ;
DeclFoncts:
       DeclFoncts DeclFonct
    |  DeclFonct
    ;
DeclFonct:
       EnTeteFonct Corps
    ;
EnTeteFonct:
       Type IDENT '(' Parametres ')'
    |  VOID IDENT '(' Parametres ')'
    ;
Parametres:
       VOID
    |  ListTypVar
    ;
ListTypVar:
       ListTypVar ',' Type IDENT
    |  Type IDENT
    ;
Corps: '{' DeclStart SuiteInstr '}'
    ;
SuiteInstr:
       SuiteInstr Instr
    |
    ;
IdentAffect : IDENT.IDENT
    | IDENT
    ;
Instr:
       IdentAffect '=' Exp ';'
    |  IF '(' Exp ')' Instr
    |  IF '(' Exp ')' Instr ELSE Instr
    |  WHILE '(' Exp ')' Instr
    |  IDENT '(' Arguments  ')' ';'
    |  RETURN Exp ';'
    |  RETURN ';'
    |  '{' SuiteInstr '}'
    |  ';'
    ;
Exp :  Exp OR TB
    |  TB
    ;
TB  :  TB AND FB
    |  FB
    ;
FB  :  FB EQ M
    |  M
    ;
M   :  M ORDER E
    |  E
    ;
E   :  E ADDSUB T
    |  T
    ;    
T   :  T DIVSTAR F 
    |  F
    ;
F   :  ADDSUB F
    |  '!' F
    |  '(' Exp ')'
    |  NUM
    |  CHARACTER
    |  IDENT
    |  IDENT '(' Arguments  ')'
    ;
Arguments:
       ListExp
    |
    ;
ListExp:
       ListExp ',' Exp
    |  Exp
    ;
%%