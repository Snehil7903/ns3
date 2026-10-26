%{
#include <stdio.h>
%}

%option noyywrap

%%
[0-9]+                  { printf("ID "); }
[a-zA-Z][a-zA-Z0-9]*    { printf("ID "); }

[+\-*/^()]              { printf("%s ", yytext); }

[ \t\n]+                ;

.                       { printf("\nInvalid character: %s\n", yytext); }
%%

int main()
{
    printf("Enter an arithmetic expression:\n");
    yylex();
    printf("\nLexical analysis completed.\n");
    return 0;
}