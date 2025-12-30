%{
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

int yylex();
void yyerror(const char *s){
    fprintf(stderr, "error : %s\n", s);
};
extern int yylineno;
Node *root;
%}

%token ADDSUB DIVSTAR EQ ORDER TYPE STRUCT OR AND IF ELSE WHILE RETURN VOID NUM CHARACTER IDENT DOT

%union{
    Node * node;
}

%type <node> Prog DeclStruct DeclStarts DeclStart Type DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr IdentAffect Instr Exp TB FB M E T F Arguments ListExp

%%
Prog: DeclStart DeclFoncts {
                                $$ = makeNode(Prog);
                                root = $$;
                                addChild($$, $1);
                                addChild($$, $2);
                            }
    ;
DeclStruct: STRUCT IDENT '{' DeclVars '}'';' {
                                                $$ = makeNode(DeclStruct);
                                                addChild($$, makeNode(TR_STRUCT));
                                                addChild($$, makeNode(TR_IDENT));
                                                addChild($$, $4);
                                            }
    ;
DeclStarts: DeclStarts DeclStart  {
                                    $$ = makeNode(DeclStarts);
                                    addSibling($$, $1);
                                    addChild($$, $2);
                                }
    | DeclStarts DeclStart  {
                                $$ = makeNode(DeclStarts);
                                addSibling($$, $1);
                                addChild($$, $2);
                            }
    ;
DeclStart : DeclVars {
            $$ = $1;
        }
    | DeclStruct {
        $$ = $1;
    }
    ; 
Type: TYPE {
                $$ = makeNode(Type);
                addChild($$, makeNode(TR_TYPE));
            }
    | STRUCT IDENT {
                    $$ = makeNode(Type);
                    addChild($$, makeNode(TR_STRUCT));
                    addChild($$, makeNode(TR_IDENT));
                }
    ;
DeclVars:
       DeclVars Type Declarateurs ';' {
                                        $$ = makeNode(DeclVars);
                                        addChild($$, $1);
                                        addChild($$, $2);
                                        addChild($$, $3);
                                    }
    ;
Declarateurs:
       Declarateurs ',' IDENT {
                                $$ = makeNode(Declarateurs);
                                addChild($$, $1);
                                addChild($$, makeNode(TR_IDENT));
                            }
    |  IDENT {
                $$ = makeNode(Declarateurs);
                addChild($$, makeNode(TR_IDENT));
            }
    ;
DeclFoncts:
       DeclFoncts DeclFonct {
                                $$ = makeNode(DeclFoncts);
                                addChild($$, $1);
                                addChild($$, $2);
                            }
    |  DeclFonct {
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
    |  VOID IDENT '(' Parametres ')' {
                                        $$ = makeNode(EnTeteFonct);
                                        addChild($$, makeNode(TR_VOID));
                                        addChild($$, makeNode(TR_IDENT));
                                        addChild($$, $4);
                                    }
    ;
Parametres:
       VOID {
                $$ = makeNode(Parametres);
                addChild($$, makeNode(TR_VOID));
            }
    |  ListTypVar {
                        $$ = makeNode(Parametres);
                        addChild($$, $1);
                    }
    ;
ListTypVar:
       ListTypVar ',' Type IDENT {
                                    $$ = makeNode(ListTypVar);
                                    addChild($$, $1);
                                    addChild($$, $3);
                                    addChild($$, makeNode(TR_IDENT));
                                }
    |  Type IDENT {
                        $$ = makeNode(ListTypVar);
                        addChild($$, $1);
                        addChild($$, makeNode(TR_IDENT));
                    }
    ;
Corps: '{' DeclStart SuiteInstr '}' {
                                        $$ = makeNode(Corps);
                                        addChild($$, $2);
                                        addChild($$, $3);
                                    }
    ;
SuiteInstr:
       SuiteInstr Instr {
                            $$ = makeNode(SuiteInstr);
                            addChild($$, $1);
                            addChild($$, $2);
                        }
    | {$$ = NULL;}
    ;
IdentAffect : IdentAffect DOT IDENT {
    $$ = makeNode(IdentAffect);
    addChild($$, $1);
    addChild($$, makeNode(TR_DOT));
    addChild($$, makeNode(TR_IDENT));
}
    | IDENT {
    $$ = makeNode(IdentAffect);
    addChild($$, makeNode(TR_IDENT));
}
    ;
Instr:
       IdentAffect '=' Exp ';' {
    $$ = makeNode(Instr);
    addChild($$, $1);
    addChild($$, $3);
}
    |  IF '(' Exp ')' Instr {
    $$ = makeNode(Instr);
    addChild($$, makeNode(TR_IF));
    addChild($$, $3);
    addChild($$, $5);
}
    |  IF '(' Exp ')' Instr ELSE Instr {
    $$ = makeNode(Instr);
    addChild($$, makeNode(TR_IF));
    addChild($$, $3);
    addChild($$, $5);
    Node* tr_else = makeNode(TR_ELSE);
    addSibling($$, tr_else);
    addChild(tr_else, $7);
}
    |  WHILE '(' Exp ')' Instr {
    $$ = makeNode(Instr);
    addChild($$, makeNode(TR_WHILE));
    addChild($$, $3);
    addChild($$, $5);
}
    |  IDENT '(' Arguments  ')' ';'{
    $$ = makeNode(Instr);
    addChild($$, makeNode(TR_IDENT));
    addChild($$, $3);
}
    |  RETURN Exp ';' {
    $$ = makeNode(Instr);
    addChild($$, makeNode(TR_RETURN));
    addChild($$, $2);
}
    |  RETURN ';'{
    $$ = makeNode(Instr);
    addChild($$, makeNode(TR_RETURN));
}
    |  '{' SuiteInstr '}'{
    $$ = makeNode(Instr);
    addChild($$, $2);
}
    |  ';'
    {
    $$ = makeNode(Instr);
}
    ;
Exp :  Exp OR TB {
    $$ = makeNode(Exp);
    addChild($$, $1);
    addChild($$, makeNode(TR_OR));
    addChild($$, $3);
}
    |  TB {
    $$ = makeNode(Exp);
    addChild($$, $1);
}
    ;
TB  :  TB AND FB {
    $$ = makeNode(TB);
    addChild($$, $1);
    addChild($$, makeNode(TR_AND));
    addChild($$, $3);
}
    |  FB {
    $$ = makeNode(TB);
    addChild($$, $1);
}
    ;
FB  :  FB EQ M {
    $$ = makeNode(FB);
    addChild($$, $1);
    addChild($$, makeNode(TR_EQ));
    addChild($$, $3);
}
    |  M {
    $$ = makeNode(FB);
    addChild($$, $1);
}
    ;
M   :  M ORDER E {
    $$ = makeNode(M);
    addChild($$, $1);
    addChild($$, makeNode(TR_ORDER));
    addChild($$, $3);
}
    |  E {
    $$ = makeNode(E);
    addChild($$, $1);
}
    ;
E   :  E ADDSUB T {
    $$ = makeNode(E);
    addChild($$, $1);
    addChild($$, makeNode(TR_ADDSUB));
    addChild($$, $3);
}
    |  T {
    $$ = makeNode(E);
    addChild($$, $1);
}
    ;    
T   :  T DIVSTAR F {
    $$ = makeNode(T);
    addChild($$, $1);
    addChild($$, makeNode(TR_DIVSTAR));
    addChild($$, $3);
}
    |  F {
    $$ = makeNode(T);
    addChild($$, $1);
}
    ;
F   :  ADDSUB F {
    $$ = makeNode(F);
    addChild($$, makeNode(TR_ADDSUB)); 
    addChild($$, $2);
}
    |  '!' F {
    $$ = makeNode(F);
    addChild($$, $2);
}
    |  '(' Exp ')' {
    $$ = makeNode(F);
    addChild($$, $2);
}
    |  NUM {
    $$ = makeNode(TR_NUM);
}
    |  CHARACTER {
    $$ = makeNode(TR_CHARACTER);
}
    |  IDENT {
    $$ = makeNode(F);
    addChild($$, makeNode(TR_IDENT));
}
    |  IDENT '(' Arguments  ')' {
    $$ = makeNode(F);
    addChild($$, makeNode(TR_IDENT));
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
    $$ = makeNode(ListExp);
    addChild($$, $1);
    addChild($$, $3);
}
    |  Exp {
        $$ = $1;
}
    ;
%%
int main(){
    printTree(root);
    deleteTree(root);
    return yyparse();
}