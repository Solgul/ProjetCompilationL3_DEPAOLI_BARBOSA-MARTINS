%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "../obj/tpc_parser.h"

int current_col = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = current_col; \
    yylloc.last_column = current_col + yyleng - 1; \
    current_col += yyleng;
%}

%option noinput nounput noyywrap yylineno

%x COMMENT

digit [0-9]
letter [a-zA-Z_]
ident {letter}({letter}|{digit})*

%%

[ \t\r]+ { /* ignore whitespace */ }
\n       { current_col = 1; }

"//".*   { /* ignore single line comment */ }

"/*"          { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n   { current_col = 1; }
<COMMENT>.    { /* ignore content inside comment */ }

struct { return STRUCT; }
if     { return IF; }
else   { return ELSE; }
while  { return WHILE; }
return { return RETURN; }
void   { return VOID; }

int    { yylval.num = 0; return TYPE; }
char   { yylval.num = 1; return TYPE; }

{ident}   { yylval.str = strdup(yytext); return IDENT; }
{digit}+  { yylval.num = atoi(yytext); return NUM; }

\'\\n\'   { yylval.num = '\n'; return CHARACTER; }
\'\\t\'   { yylval.num = '\t'; return CHARACTER; }
\'\\\\\'  { yylval.num = '\\'; return CHARACTER; }
\'\\'\'   { yylval.num = '\''; return CHARACTER; }
\'[^'\\]\' { yylval.num = (int)yytext[1]; return CHARACTER; }

"==" { yylval.num = 0; return EQ; }
"!=" { yylval.num = 1; return EQ; }
"<"  { yylval.num = 0; return ORDER; }
"<=" { yylval.num = 1; return ORDER; }
">"  { yylval.num = 2; return ORDER; }
">=" { yylval.num = 3; return ORDER; }

"&&" { return AND; }
"||" { return OR; }

"=" { return '='; }
"+" { yylval.num = 0; return ADDSUB; }
"-" { yylval.num = 1; return ADDSUB; }

"*" { yylval.num = 0; return DIVSTAR; }
"/" { yylval.num = 1; return DIVSTAR; }
"%" { yylval.num = 2; return DIVSTAR; }

"!" { return '!'; }
";" { return ';'; }
"," { return ','; }
"(" { return '('; }
")" { return ')'; }
"{" { return '{'; }
"}" { return '}'; }
"." { return '.'; }

. {
    fprintf(stderr, "Caractère illégal ligne %d col %d: '%s'\n", yylineno, current_col, yytext);
    return 0;
}

%%
