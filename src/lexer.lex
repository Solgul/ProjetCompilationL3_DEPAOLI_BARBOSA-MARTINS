%{

%}
%x COMM
%option nounput
%option noinput
%%

[_a-zA-Z][_a-za-z0-9]*   return IDENT;
[a-zA-Z]+                return CHARACTER;
[0-9]+                   return NUM;
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

"//".* ;
"/*" BEGIN COMM;


<COMM>"*/" BEGIN INITIAL;
<COMM>[.\n] ;
%%
