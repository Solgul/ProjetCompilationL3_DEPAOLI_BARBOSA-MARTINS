%{
#include <stdio.h>
#include <string.h>
#include "tree.h"
#include "../obj/parser.h"

int lineno = 1;
int colno = 1;
#define YY_USER_ACTION colno += yyleng;
%}

%x COMM

%option noyywrap
%option nounput
%option noinput

%%
        /* ===================== */
        /* ESPACES ET RETOURS   */
        /* ===================== */
[ \t\r]+                 ;
\n                       { lineno++; colno = 1; }

        /* ===================== */
        /* COMMENTAIRES          */
        /* ===================== */
"//".*                   ;
"/*"                     BEGIN(COMM);
<COMM>"*/"               BEGIN(INITIAL);
<COMM>\n                 {lineno++; colno = 1;}
<COMM>.                  ;

        /* ===================== */
        /* MOTS-CLÉS             */
        /* ===================== */
"if"                     return IF;
"else"                   return ELSE;
"return"                 return RETURN;
"while"                  return WHILE;
"int"|"char"             { yylval.str = strdup(yytext); return TYPE; }
"void"                   return VOID;
"struct"                 return STRUCT;

        /* ===================== */
        /* OPÉRATEURS            */
        /* ===================== */
"=="|"!="                { yylval.str = strdup(yytext); return EQ; }
"<="|">="|"<"|">"        { yylval.str = strdup(yytext); return ORDER; }
"&&"                     return AND;
"||"                     return OR;
"+"|"-"                  { yylval.str = strdup(yytext); return ADDSUB; }
"*"|"/"|"%"              { yylval.str = strdup(yytext); return DIVSTAR; }
"!"                      return NOT;
"="                      return '=';

        /* ===================== */
        /* SÉPARATEURS           */
        /* ===================== */
";"                      return ';';
","                      return ',';
"."                      return DOT;
"("                      return '(';
")"                      return ')';
"{"                      return '{';
"}"                      return '}';

        /* ===================== */
        /* CONSTANTES            */
        /* ===================== */
[0-9]+                   { yylval.num = atoi(yytext); return NUM; }

\'(\\[nt\\\']|[^\\'])\'  { yylval.str = strdup(yytext); return CHARACTER; }

        /* ===================== */
        /* IDENTIFICATEURS       */
        /* ===================== */
[_a-zA-Z][_a-zA-Z0-9]* { yylval.str = strdup(yytext); return IDENT; }

        /* ===================== */
        /* ERREURS LEXICALES     */
        /* ===================== */
. {
    fprintf(stderr, "Erreur lexicale ligne %d, colonne %d : caractère invalide '%s'\n", lineno, colno, yytext);
    return 1;
}
%%