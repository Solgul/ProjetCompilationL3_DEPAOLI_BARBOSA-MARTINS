%{
#include <stdio.h>
#include <string.h>
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
"int"|"char"             { yylval.str = strdup(yytext); return TYPE; }
"void"                   return VOID;

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
"("                      return '(';
")"                      return ')';
"{"                      return '{';
"}"                      return '}';

        /* ===================== */
        /* CONSTANTES            */
        /* ===================== */
[0-9]+                   { yylval.num = atoi(yytext); return NUM; }

        /* caractère littéral : 'a', '\n', '\t', '\'' */
\'(\\[nt\\\']|[^\\'])\'  { yylval.str = strdup(yytext); return CHARACTER; }

        /* ===================== */
        /* IDENTIFICATEURS       */
        /* ===================== */
[_a-zA-Z][_a-zA-Z0-9]*   { yylval.str = strdup(yytext); return IDENT; }

        /* ===================== */
        /* ERREURS LEXICALES     */
        /* ===================== */
. {
    fprintf(stderr,
        "Erreur lexicale ligne %d : caractère invalide '%s'\n",
        lineno, yytext);
}
%%