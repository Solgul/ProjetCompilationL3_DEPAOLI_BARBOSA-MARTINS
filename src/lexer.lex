%{

%}
%x COMM ONELINECOMM FUNC COND
%option nounput
%option noinput
%%

[_a-zA-Z][_a-za-z0-9]*   return IDENT;
[a-zA-Z]+       return CHARACTER;
[0-9]+          return NUM;
[+-]            return ADDSUB;
[*/%]           return DIVSTAR;
[{==}{!=}]      return EQ;
[<{<=}{>=}>]    return ORDER;
"||"            return OR;
"&&"            return AND;
"if"            return IF;
"else"          return ELSE;
"return"        return RETURN;

"//" BEGIN ONELINECOMM;
"/*" BEGIN COMM;

"{" BEGIN FUNC;

"(" BEGIN COND;

<ONELINECOMM>\n BEGIN INITIAL;
<ONELINECOMM>. ;

<COMM> "*/" BEGIN INITIAL;
<COMM>[.\n] ;

<FUNC>"}" BEGIN INITIAL;
<FUNC>[.\n] ;

<COND>")" BEGIN INITIAL;
<COND>[.\n] ;
%%
