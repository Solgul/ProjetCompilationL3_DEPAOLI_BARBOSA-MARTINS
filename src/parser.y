%{
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

int yylex();
void yyerror(const char *s){
    fprintf(stderr, "error : %s\n", s);
};
extern int yylineno;
Node *root = NULL;
%}

%union{
    Node* node;
}

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left OR
%left AND
%left EQ
%left ORDER
%left ADDSUB
%left DIVSTAR

%token <node> ADDSUB DIVSTAR EQ ORDER TYPE STRUCT OR AND IF ELSE WHILE RETURN VOID NUM CHARACTER IDENT DOT NOT

%type <node> Prog DeclStarts DeclStart Type DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr IdentAffect Instr Exp TB FB M E T F Arguments ListExp

%%
Prog :
    DeclStarts DeclFoncts {
                        fprintf(stderr,"5");
                        $$ = makeNode(Prog);
                        if($1 != NULL){
                            addChild($$, $1);
                        }
                        addChild($$, $2);
                        fprintf(stderr,"6");
                        printTree($$);
                        deleteTree($$);
    }
;

DeclStart: 
    DeclVars {
            $$ = $1;
    }
    | STRUCT IDENT '{' DeclVars '}'';' {
                                $$ = makeNode(DeclStruct);
                                addChild($$, makeNode(TR_STRUCT));
                                addChild($$, makeNode(TR_IDENT));
                                addChild($$, $4);
    }
;

DeclStarts: 
    DeclStarts DeclStart  {
                        $$ = makeNode(DeclStarts);
                        addChild($$, $1);
                        addChild($$, $2);
    }
    | {
        $$ = NULL;
    }
;

Type: 
    TYPE {
        $$ = makeNode(TR_TYPE);
    }
    | STRUCT IDENT {
                $$ = makeNode(TR_STRUCT);
                addChild($$, makeNode(TR_IDENT));
    }
;

DeclVars : 
    Type Declarateurs ';' {
                    $$ = makeNode(DeclVars);
                    addChild($$, $1);
                    addChild($$, $2);
        }
;

Declarateurs:
    Declarateurs ',' IDENT {
                    $$ = $1;
                    $$ = makeNode(Declarateurs);
                    addChild($$, makeNode(TR_IDENT));
    }
    | IDENT {
            $$ = makeNode(Declarateurs);
            addChild($$, makeNode(TR_IDENT));
    }
;

DeclFoncts:
    DeclFoncts DeclFonct {
                            $$ = $1;
                            addChild($$, $2);
    }
    | DeclFonct {
                $$ = makeNode(DeclFoncts);
                addChild($$, $1);
    }
;

DeclFonct:
    EnTeteFonct Corps {
                        $$ = makeNode(DeclFonct);
                        addChild($$, $1);
                        addChild($$, $2);
    }
;

EnTeteFonct:
    Type IDENT '(' Parametres ')' {
                                    $$ = makeNode(EnTeteFonct);
                                    addChild($$, $1);
                                    addChild($$, makeNode(TR_IDENT));
                                    addChild($$, $4);
    }
    | VOID IDENT '(' Parametres ')' {
                                    $$ = makeNode(EnTeteFonct);
                                    addChild($$, makeNode(TR_VOID));
                                    addChild($$, makeNode(TR_IDENT));
                                    addChild($$, $4);
    }
;

Parametres:
    VOID {
            $$ = makeNode(TR_VOID);
    }
    | ListTypVar {
                $$ = $1; 
    }
    | { 
        $$ = makeNode(TR_VOID);  /* vide = void implicite */ 
    }
;

ListTypVar:
    ListTypVar ',' Type IDENT {
                        $$ = $1;
                        addChild($$, $3);
                        addChild($$, makeNode(TR_IDENT));
    }
    |  Type IDENT {
                $$ = makeNode(ListTypVar);
                addChild($$, $1);
                addChild($$, makeNode(TR_IDENT));
    }
;

Corps:
    '{' DeclStarts SuiteInstr '}' {
                            $$ = makeNode(Corps);
                            if($2 != NULL){
                                addChild($$, $2);
                            }
                            addChild($$, $3);
    }
;

SuiteInstr:
    SuiteInstr Instr {
                    $$ = $1;
                    addChild($$, $2);
    }
    | Instr {
        $$ = $1;
    }
;

IdentAffect :
    IdentAffect DOT IDENT {
                    $$ = makeNode(TR_DOT);
                    addChild($$, $1);
                    addChild($$, makeNode(TR_IDENT));
    }
    | IDENT {
        $$ = makeNode(TR_IDENT);
    }
;

Instr:
    IdentAffect '=' Exp ';' {
            $$ = makeNode(IdentAffect);
            addChild($$, $1);
            addChild($$, $3);
    }
    | IF '(' Exp ')' Instr %prec LOWER_THAN_ELSE {
            $$ = makeNode(TR_IF);
            addChild($$, $3);
            addChild($$, $5);
    }
    | IF '(' Exp ')' Instr ELSE Instr {
            $$ = makeNode(TR_IF);
            addChild($$, $3);
            addChild($$, $5);
            Node* tr_else = makeNode(TR_ELSE);
            addSibling($$, tr_else);
            addChild(tr_else, $7);
    }
    | WHILE '(' Exp ')' Instr {
            $$ = makeNode(TR_WHILE);
            addChild($$, $3);
            addChild($$, $5);
    }
    | IDENT '(' Arguments  ')' ';' {
            $$ = makeNode(Instr);
            addChild($$, makeNode(TR_IDENT));
            addChild($$, $3 ? $3 : makeNode(Arguments));
    }
    | RETURN Exp ';' {
            $$ = makeNode(TR_RETURN);
            addChild($$, $2);
    }
    | RETURN ';' {
            $$ = makeNode(TR_RETURN);
    }
    | '{' SuiteInstr '}' {
            $$ = makeNode(Instr);
            addChild($$, $2 ? $2 : makeNode(SuiteInstr));
    }
    | ';'{
            $$ = makeNode(Instr);
    }
;

Exp :  
    Exp OR TB {
            $$ = makeNode(TR_OR);
            addChild($$, $1);
            addChild($$, $3);
    }
    | TB {
            $$ = $1;
    }
;

TB :
    TB AND FB {
            $$ = makeNode(TR_AND);
            addChild($$, $1);
            addChild($$, $3);
    }
    | FB {
            $$ = $1;
    }
;

FB :  
    FB EQ M {
            $$ = makeNode(TR_EQ);
            addChild($$, $1);
            addChild($$, $3);
    }
    | M {
            $$ = $1;
    }
;

M : 
    M ORDER E {
            $$ = makeNode(TR_ORDER);
            addChild($$, $1);
            addChild($$, $3);
    }
    | E {
            $$ = $1;
    }
;

E : 
    E ADDSUB T {
            $$ = makeNode(TR_ADDSUB);
            addChild($$, $1);
            addChild($$, $3);
    }
    | T {
            $$ = $1;
    }
;

T : 
    T DIVSTAR F {
            $$ = makeNode(TR_DIVSTAR);
            addChild($$, $1);
            addChild($$, $3);
    }
    |  F {
            $$ = $1;
    }
;

F : 
    ADDSUB F {
            $$ = makeNode(TR_ADDSUB);
            addChild($$, $2);
    }
    | '!' F {
            $$ = makeNode(TR_NOT);
            addChild($$, $2);
    }
    | '(' Exp ')' {
            $$ = $2;
    }
    | NUM {
            $$ = makeNode(TR_NUM);
    }
    | CHARACTER {
            $$ = makeNode(TR_CHARACTER);
    }
    | IdentAffect {
            $$ = $1;
    }
    | IDENT '(' Arguments  ')' {
            $$ = makeNode(TR_IDENT);
            addChild($$, $3);
    }
;

Arguments:
       ListExp {
            $$ = $1;
    }
    | {
    $$ = makeNode(Arguments);
    }
;

ListExp:
    ListExp ',' Exp {
            $$ = $1;
            addChild($$, $3);
    }
    | Exp {
        $$ = makeNode(ListExp);
        addChild($$, $1);
    }
;

%%
int main(){
    fprintf(stderr, "1\n");
    int res = yyparse();
    fprintf(stderr, "2\n");
    if (res == 0 && root != NULL) {
        fprintf(stderr, "3\n");
        printTree(root);
        deleteTree(root);
    }
    return res;
}