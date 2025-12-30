%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tree.h"

int yylex();
extern int lineno;
extern char *yytext; // Utile pour voir sur quel mot ça plante

void yyerror(const char *s) {
    fprintf(stderr, "Erreur syntaxique ligne %d : %s\n",
            lineno, s);
}

Node *root = NULL;
int print_tree = 0;
%}

%union{
    Node* node;
    int num;
    char* str;
}

/* Priorités (Attention à l'ordre, du plus faible au plus fort) */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left OR
%left AND
%left EQ
%left ORDER
%left ADDSUB
%left DIVSTAR
%nonassoc UMINUS NOT /* UMINUS pour le moins unaire si besoin */

/* Tokens */
%token <node> ADDSUB DIVSTAR EQ ORDER TYPE STRUCT OR AND IF ELSE WHILE RETURN VOID NUM CHARACTER IDENT DOT NOT

/* Types */
%type <node> Prog GlobalDecls GlobalDecl DeclStruct ListDeclVars DeclStarts DeclStart Type DeclVars Declarateurs DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr IdentAffect Instr Exp TB FB M E T F Arguments ListExp

%%

/* ========================================================================= */
/* PROGRAMME                                                                 */
/* ========================================================================= */

Prog :
    GlobalDecls {
        $$ = makeNode(Prog);
        if($1 != NULL) addChild($$, $1);
        root = $$;
    }
;

GlobalDecls :
    GlobalDecls GlobalDecl {
        $$ = $1;
        addChild($$, $2);
    }
    | /* vide */ {
        $$ = makeNode(DeclStarts);
    }
;

GlobalDecl :
    DeclVars { $$ = $1; }
    | DeclStruct { $$ = $1; }
    | DeclFonct { $$ = $1; }
;

/* ========================================================================= */
/* DÉCLARATIONS                                                              */
/* ========================================================================= */

DeclStruct:
    STRUCT IDENT '{' ListDeclVars '}' ';' {
        $$ = makeNode(DeclStruct);
        addChild($$, makeNode(TR_STRUCT));
        addChild($$, makeNode(TR_IDENT));
        if ($4 != NULL) addChild($$, $4);
    }
;

ListDeclVars:
    ListDeclVars DeclVars {
        $$ = $1;
        addChild($$, $2);
    }
    | /* vide */ {
        $$ = makeNode(DeclVars);
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
        addChild($$, makeNode(TR_IDENT));
    }
    | IDENT {
        $$ = makeNode(Declarateurs);
        addChild($$, makeNode(TR_IDENT));
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

/* ========================================================================= */
/* FONCTIONS                                                                 */
/* ========================================================================= */

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
    |
    VOID IDENT '(' Parametres ')' {
        $$ = makeNode(EnTeteFonct);
        addChild($$, makeNode(TR_VOID));
        addChild($$, makeNode(TR_IDENT));
        addChild($$, $4);
    }
;

Parametres:
    VOID { $$ = makeNode(TR_VOID); }
    | ListTypVar { $$ = $1; }
    | /* vide */ { $$ = makeNode(TR_VOID); }
;

ListTypVar:
    ListTypVar ',' Type IDENT {
        $$ = $1;
        addChild($$, $3);
        addChild($$, makeNode(TR_IDENT));
    }
    | Type IDENT {
        $$ = makeNode(ListTypVar);
        addChild($$, $1);
        addChild($$, makeNode(TR_IDENT));
    }
;

Corps:
    '{' DeclStarts SuiteInstr '}' {
        $$ = makeNode(Corps);
        /* L'ordre est strict : Déclarations PUIS Instructions */
        if($2 != NULL) addChild($$, $2);
        if($3 != NULL) addChild($$, $3);
    }
;

DeclStarts: 
    DeclStarts DeclStart {
        $$ = $1;
        addChild($$, $2);
    }
    | /* vide */ {
        $$ = NULL; 
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
        $$ = $1;
        addChild($$, $2);
    }
    | /* vide */ { 
        /* CHANGE : Permettre un corps de fonction sans instructions */
        $$ = NULL; 
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
        Node* tr_else;
        $$ = makeNode(TR_IF);
        addChild($$, $3);    
        addChild($$, $5);    
        
        tr_else = makeNode(TR_ELSE);
        addChild($$, tr_else); 
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
        if($2 != NULL) addChild($$, $2);
    }
    | ';' {
        /* Instruction vide */
        $$ = makeNode(Instr);
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

/* ========================================================================= */
/* EXPRESSIONS                                                               */
/* ========================================================================= */

Exp :  
    Exp OR TB {
        $$ = makeNode(TR_OR);
        addChild($$, $1);
        addChild($$, $3);
    }
    | TB { $$ = $1; }
;

TB :
    TB AND FB {
        $$ = makeNode(TR_AND);
        addChild($$, $1);
        addChild($$, $3);
    }
    | FB { $$ = $1; }
;

FB :  
    FB EQ M {
        $$ = makeNode(TR_EQ);
        addChild($$, $1);
        addChild($$, $3);
    }
    | M { $$ = $1; }
;

M : 
    M ORDER E {
        $$ = makeNode(TR_ORDER);
        addChild($$, $1);
        addChild($$, $3);
    }
    | E { $$ = $1; }
;

E : 
    E ADDSUB T {
        $$ = makeNode(TR_ADDSUB);
        addChild($$, $1);
        addChild($$, $3);
    }
    | T { $$ = $1; }
;

T : 
    T DIVSTAR F {
        $$ = makeNode(TR_DIVSTAR);
        addChild($$, $1);
        addChild($$, $3);
    }
    | F { $$ = $1; }
;

F : 
    ADDSUB F {
        /* Moins unaire ou Plus unaire */
        $$ = makeNode(TR_ADDSUB);
        addChild($$, $2);
    }
    | NOT F {
        $$ = makeNode(TR_NOT);
        addChild($$, $2);
    }
    | '(' Exp ')' { $$ = $2; }
    | NUM { $$ = makeNode(TR_NUM); }
    | CHARACTER { $$ = makeNode(TR_CHARACTER); }
    | IdentAffect { $$ = $1; }
    | IDENT '(' Arguments  ')' {
        $$ = makeNode(TR_IDENT);
        addChild($$, $3);
    }
;

Arguments:
    ListExp { $$ = $1; }
    | /* vide */ { $$ = makeNode(Arguments); }
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

void print_help(const char *progname) {
    printf("Usage: %s [options] < [path]\n", progname);
    printf("Options:\n");
    printf("  -t        Affiche l'arbre syntaxique (AST)\n");
    printf("  -h        Affiche cette aide\n");
}

int main(int argc, char **argv){
    int opt;
    //yydebug = 1; // Décommentez pour un déluge d'informations de debug
    
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

    int res = yyparse();
    
    if (res == 0 && root != NULL) {
        if (print_tree) {
            printTree(root);
        }
        deleteTree(root);
    }
    
    return res;
}