%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "tree.h"
#include "sym.h"
#include "asm.h"

int yylex();
extern int lineno;
extern FILE *yyin;
extern char *yytext;
void yyerror(const char *s) {
    fprintf(stderr, "Erreur syntaxique ligne %d : %s\n", lineno, s);
}

Node *root = NULL;
int print_tree = 0;
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

%type <node> Prog GlobalDecls GlobalDecl DeclStruct ListDeclVars DeclStarts DeclStart 
%type <node> Type DeclVars Declarateurs DeclFonct EnTeteFonct Parametres ListTypVar 
%type <node> Corps SuiteInstr IdentAffect Instr Exp TB FB M E T F Arguments ListExp

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
        if ($1->label == Ident) {
            $$->ident = $1->ident;
            free($1);
        } else {
            addChild($$, $1);
        }
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

void print_help(const char *exeName) {
    printf("Usage: %s [OPTIONS] [FILE.tpc]\n", exeName);
    printf("Options:\n");
    printf("  -t, --tree    Affiche l'arbre abstrait sur la sortie standard\n");
    printf("  -h, --help \n");
    printf("\n");
}

int handle_opt(int argc, char *argv[]) {
    int opt; int val = 0;
    static struct option long_options[] = {
        {"tree", no_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "th", long_options, NULL)) != -1) {
        switch (opt) {
        case 't':
            val = 1;
            break;
        case 'h':
            print_help(argv[0]);
            return 2;
        default:
            fprintf(stderr, "Option incorrecte.\n");
            print_help(argv[0]);
            return 3;
        }
    }

    if(optind < argc){
        FILE *f = fopen(argv[optind], "r");
        if(!f){
            fprintf(stderr, "votre fichier est invalide");
            return 2;
        }
        yyin = f;
    }

    return val;
}

int main(int argc, char **argv){
    yyin = stdin;
    int opt = handle_opt(argc, argv);
    if(opt == 2){ //help or error
        return 1;
    }

    int res = yyparse();
    
    if (res == 0 && root != NULL) {
        if (opt == 1) {
            printTree(root);
        }
        SymbolTable *globalTable = buildSymbolTable(root);
        int semErrors = checkSemantics(root, globalTable);
        if (semErrors > 0) {
            res = 2;
        }
        FILE * file = fopen("asm/_anonymous.asm", "w+");
        if (!file) {
            fprintf(stderr, "Couldn't create the .asm file\n");
            return -1;
        }
        parcours_depart(root, file, globalTable);
        fclose(file);
        deleteTree(root);
    }
    if(yyin != stdin){
        fclose(yyin);
    }

    return res;
}