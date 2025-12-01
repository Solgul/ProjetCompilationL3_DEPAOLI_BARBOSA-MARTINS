%{

%}
%x COMM
%option nounput
%option noinput
%%

[+-]                     return ADDSUB;
[*/%]                    return DIVSTAR;
[{==}{!=}]               return EQ;
[<{<=}{>=}>]             return ORDER;
"||"                     return OR;
"&&"                     return AND;
"if"                     return IF;
"else"                   return ELSE;
"return"                 return RETURN;
"void"                   return VOID;
[({});,=]                return yytext[0];
[0-9]+                   return NUM;
[a-zA-Z]+                return CHARACTER;
[_a-zA-Z][_a-zA-Z0-9]*   return IDENT;

"//".* ;
"/*" BEGIN COMM;


<COMM>"*/" BEGIN INITIAL;
<COMM>[.\n] ;
%%
