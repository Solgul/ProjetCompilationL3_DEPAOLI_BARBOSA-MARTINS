%{
#include <stdio.h>
#include "tree.h"
#include "../obj/parser.h"   /* tokens Bison */
int lineno = 1;
%}

/* état pour les commentaires multi-lignes */
%x COMM

%option noyywrap
%option nounput
%option noinput

%%
        /* ===================== */
        /* ESPACES ET RETOURS   */
        /* ===================== */
[ \t\r]+                 ;
\n                       { lineno++; }

        /* ===================== */
        /* COMMENTAIRES          */
        /* ===================== */
"//".*                   ;              /* commentaire ligne */
"/*"                     BEGIN(COMM);

<COMM>"*/"               BEGIN(INITIAL);
<COMM>.|\n               ;

        /* ===================== */
        /* MOTS-CLÉS             */
        /* ===================== */
"if"                     return IF;
"else"                   return ELSE;
"return"                 return RETURN;
"while"                  return WHILE;
"int"|"char"             return TYPE;
"void"                   return VOID;

        /* ===================== */
        /* OPÉRATEURS            */
        /* ===================== */
"=="|"!="                return EQ;
"<="|">="|"<"|">"        return ORDER;
"&&"                     return AND;
"||"                     return OR;
"+"|"-"                  return ADDSUB;
"*"|"/"|"%"              return DIVSTAR;
"!"                      return NOT;
"="                      return '=';

        /* ===================== */
        /* SÉPARATEURS           */
        /* ===================== */
";"                      return ';';
","                      return ',';
"("                      return '(';
")"                      return ')';
"{"                      return '{';
"}"                      return '}';

        /* ===================== */
        /* CONSTANTES            */
        /* ===================== */
[0-9]+                   return NUM;

        /* caractère littéral : 'a', '\n', '\t', '\'' */
\'(\\[nt\\\']|[^\\'])\'  return CHARACTER;

        /* ===================== */
        /* IDENTIFICATEURS       */
        /* ===================== */
[_a-zA-Z][_a-zA-Z0-9]*   return IDENT;

        /* ===================== */
        /* ERREURS LEXICALES     */
        /* ===================== */
. {
    fprintf(stderr,
        "Erreur lexicale ligne %d : caractère invalide '%s'\n",
        lineno, yytext);
}
%%