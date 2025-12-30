%{
#include "tree.h"
#include "../obj/parser.h"
int lineno = 1;
extern int yylineno;
%}
%x COMM
%option nounput
%option noinput
%option noyywrap
%%

[ \t\r]                  ;
[{==}{!=}]               return EQ;
[<{<=}{>=}>]             return ORDER;
[+-]                     return ADDSUB;
[*/%]                    return DIVSTAR;
"||"                     return OR;
"&&"                     return AND;
"int"|"char"             return TYPE;
"struct"                 return STRUCT;
"void"                   return VOID;
"if"                     return IF;
"else"                   return ELSE;
"while"                  return WHILE;
"return"                 return RETURN;
[0-9]+                   return NUM;
[a-zA-Z]+                return CHARACTER;
[_a-zA-Z][_a-zA-Z0-9]*   return IDENT;
[({});,=!]               return yytext[0];
"."                      return DOT;
"!"                      return NOT;
[\n]                     ;


"//".* ;
"/*" BEGIN COMM;


<COMM>"*/" BEGIN INITIAL;
<COMM>[.\n] ;
%%
