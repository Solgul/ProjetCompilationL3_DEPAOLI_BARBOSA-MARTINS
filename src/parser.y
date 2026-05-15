%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "tree.h"

int yylex();
extern int lineno;
extern int colno;
void yyerror(const char *s) {
    fprintf(stderr, "Erreur syntaxique ligne %d, colonne %d : %s\n", lineno, colno, s);
}

Node *root = NULL;
%}

%union {
    Node* node;
    int num;
    char* str;
}

%token <str> IDENT TYPE CHARACTER ADDSUB DIVSTAR EQ ORDER 
%token <num> NUM

%token STRUCT OR AND IF ELSE WHILE RETURN VOID DOT NOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left OR
%left AND
%left EQ
%left ORDER
%left ADDSUB
%left DIVSTAR
%nonassoc UMINUS NOT

%type <node> Prog GlobalDecls GlobalDecl DeclFoncts DeclStruct ListDeclVars DeclStarts DeclStart 
%type <node> Type DeclVars Declarateurs DeclFonct EnTeteFonct Parametres ListTypVar 
%type <node> Corps SuiteInstr IdentAffect Instr Exp TB FB M E T F Arguments ListExp

%%

/* ========================================================================= */
/* PROGRAMME                                                                 */
/* ========================================================================= */

Prog :
    GlobalDecls DeclFoncts {
        $$ = makeNode(Prog);
        if($1 != NULL) addChild($$, $1);
        if($2 != NULL) addChild($$, $2);
        root = $$;
    }
;

GlobalDecls :
    GlobalDecls GlobalDecl {
        if ($1 == NULL) {
            $$ = $2;
        } else {
            $$ = $1;
            addSibling($$, $2);
        }
    }
    |{
        $$ = NULL;
    }
;

GlobalDecl :
    DeclVars { $$ = $1; }
    | DeclStruct { $$ = $1; }
;

DeclFoncts :
    DeclFoncts DeclFonct {
        $$ = $1;
        addSibling($$, $2);
    }
    | DeclFonct { $$ = $1; }
;

/* ========================================================================= */
/* DÉCLARATIONS (Variables et Structures)                                    */
/* ========================================================================= */

DeclStruct:
    STRUCT IDENT '{' ListDeclVars '}' ';' {
        $$ = makeNode(DeclStruct);
        $$->ident = $2;
        if ($4 != NULL) addChild($$, $4);
    }
;

ListDeclVars:
    ListDeclVars DeclVars {
        $$ = $1;
        addChild($$, $2);
    }
    | DeclVars {
        $$ = makeNode(DeclVars);
        addChild($$, $1);
    }
;

DeclVars : 
    Type Declarateurs ';' {
        $$ = $1;
        addChild($$, $2);
    }
;

Declarateurs:
    Declarateurs ',' IDENT {
        $$ = $1;
        Node *id = makeNode(Ident); 
        id->ident = $3;               
        addSibling($$, id);
    }
    | IDENT {
        $$ = makeNode(Ident);
        $$->ident = $1;
    }
;

Type: 
    TYPE {
        $$ = makeNode(Type);
        $$->ident = $1;
    }
    | STRUCT IDENT {
        $$ = makeNode(Struct);
        $$->ident = $2; 
    }
;

/* ========================================================================= */
/* FONCTIONS                                                                 */
/* ========================================================================= */

DeclFonct:
    EnTeteFonct Corps {
        $$ = makeNode(DeclFonct);
        addChild($$, $1);
        if ($2 != NULL) addChild($$, $2);
    }
;

EnTeteFonct:
    Type IDENT '(' Parametres ')' {
        $$ = makeNode(EnTeteFonct);
        addChild($$, $1);
        Node *id = makeNode(Ident);
        id->ident = $2; 
        addChild($$, id);
        
        Node *var = makeNode(Var);
        addChild(var, $4);
        addChild($$, var);
    }
    | VOID IDENT '(' Parametres ')' {
        $$ = makeNode(EnTeteFonct);
        addChild($$, makeNode(Void));
        Node *id = makeNode(Ident);
        id->ident = $2; 
        addChild($$, id);
        
        Node *var = makeNode(Var);
        addChild(var, $4);
        addChild($$, var);
    }
;

Parametres:
    VOID { $$ = makeNode(Void); }
    | ListTypVar { $$ = $1; }
    |{ $$ = makeNode(Void); }
;

ListTypVar:
    ListTypVar ',' Type IDENT {
        $$ = $1;
        Node* t = $3;
        Node *id = makeNode(Ident);
        id->ident = $4; 
        addChild(t, id);
        addSibling($$, t);
    }
    | Type IDENT {
        $$ = $1;
        Node *id = makeNode(Ident);
        id->ident = $2;
        addChild($$, id);
    }
;

Corps:
    '{' DeclStarts SuiteInstr '}' {
        $$ = $2;
        Node* instr = makeNode(Instr);
        addSibling($$, instr);
        if ($3 != NULL) addChild(instr, $3);
    }
;

DeclStarts: 
    DeclStarts DeclStart {
        if ($1 == NULL) {
            $$ = makeNode(DeclVars);
            addChild($$, $2);
        } else {
            $$ = $1;
            addChild($$, $2);
        }
    }
    |{
        $$ = makeNode(DeclVars);
    }
;

DeclStart: 
    DeclVars { $$ = $1; }
    | DeclStruct { $$ = $1; }
;

/* ========================================================================= */
/* INSTRUCTIONS                                                              */
/* ========================================================================= */

SuiteInstr:
    SuiteInstr Instr {
        if ($1 != NULL) {
            $$ = $1;
            addSibling($$, $2);
        } else {
            $$ = $2;
        }
    }
    |{ 
        $$ = NULL;
    }
;

Instr:
    IdentAffect '=' Exp ';' {
        $$ = makeNode(Affect);
        addChild($$, $1);
        addChild($$, $3);
    }
    | IF '(' Exp ')' Instr %prec LOWER_THAN_ELSE {
        $$ = makeNode(If);
        Node* cond = makeNode(Cond);
        addChild(cond, $3);
        addChild($$, cond);
        if ($5 != NULL) {
            Node* doo = makeNode(Do);
            addChild(doo, $5);
            addChild($$, doo);
        }
    }
    | IF '(' Exp ')' Instr ELSE Instr {
        $$ = makeNode(If);
        Node* cond = makeNode(Cond);
        addChild(cond, $3);
        addChild($$, cond);
        if ($5 != NULL) {
            Node* doo = makeNode(Do);
            addChild(doo, $5);
            addChild($$, doo);
        }
        if ($7 != NULL) {
            Node* elsee = makeNode(Else);
            addChild(elsee, $7);
            addChild($$, elsee);
        }
    }
    | WHILE '(' Exp ')' Instr {
        $$ = makeNode(While);
        Node* cond = makeNode(Cond);
        addChild(cond, $3);
        addChild($$, cond);
        if ($5 != NULL) {
            Node* doo = makeNode(Do);
            addChild(doo, $5);
            addChild($$, doo);
        }
    }
    | IDENT '(' Arguments ')' ';' {
        $$ = makeNode(Funct);
        $$->ident = $1;
        if ($3 != NULL) addChild($$, $3);
    }
    | RETURN Exp ';' {
        $$ = makeNode(Return);
        addChild($$, $2);
    }
    | RETURN ';' {
        $$ = makeNode(Return);
    }
    | '{' SuiteInstr '}' {
        $$ = $2;
    }
    | ';' {
        $$ = NULL;
    }
;

IdentAffect :
    IdentAffect DOT IDENT {
        $$ = makeNode(Dot);
        addChild($$, $1);
        Node *id = makeNode(Ident);
        id->ident = $3; 
        addChild($$, id);
    }
    | IDENT {
        $$ = makeNode(Ident);
        $$->ident = $1; 
    }
;

/* ========================================================================= */
/* EXPRESSIONS                                                               */
/* ========================================================================= */

Exp :  
    Exp OR TB {
        $$ = makeNode(Or);
        addChild($$, $1);
        addChild($$, $3);
    }
    | TB { $$ = $1; }
;

TB :
    TB AND FB {
        $$ = makeNode(And);
        addChild($$, $1);
        addChild($$, $3);
    }
    | FB { $$ = $1; }
;

FB :  
    FB EQ M {
        $$ = makeNode(Equal);
        $$->ident = $2;
        addChild($$, $1);
        addChild($$, $3);
    }
    | M { $$ = $1; }
;

M : 
    M ORDER E {
        $$ = makeNode(Order);
        $$->ident = $2; 
        addChild($$, $1);
        addChild($$, $3);
    }
    | E { $$ = $1; }
;

E : 
    E ADDSUB T {
        $$ = makeNode(Addsub);
        $$->ident = $2;
        addChild($$, $1);
        addChild($$, $3);
    }
    | T { $$ = $1; }
;

T : 
    T DIVSTAR F {
        $$ = makeNode(Divstar);
        $$->ident = $2;
        addChild($$, $1);
        addChild($$, $3);
    }
    | F { $$ = $1; }
;

F : 
    ADDSUB F {
        $$ = makeNode(UnOperator);
        $$->ident = $1;
        addChild($$, $2);
    }
    | NOT F {
        $$ = makeNode(Negate);
        addChild($$, $2);
    }
    | '(' Exp ')' { $$ = $2; }
    | NUM { 
        $$ = makeNode(Num); 
        $$->num = $1;
    }
    | CHARACTER { 
        $$ = makeNode(Char);
        $$->ident = $1; 
    }
    | IdentAffect { $$ = $1; }
    | IDENT '(' Arguments ')' {
        $$ = makeNode(Funct);
        $$->ident = $1;
        if ($3 != NULL) addChild($$, $3);
    }
;

Arguments:
    ListExp { $$ = $1; }
    |{ $$ = NULL; }
;

ListExp:
    ListExp ',' Exp {
        $$ = $1;
        addSibling($$, $3);
    }
    | Exp {
        $$ = $1;
    }
;

%%