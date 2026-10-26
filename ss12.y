%{
#include <stdio.h>
%}

%%
da      { printf("Valid string\n"); }
bdc     { printf("Valid string\n"); }
dc      { printf("Valid string\n"); }
bda     { printf("Valid string\n"); }

[ \t\n]+    ;
.           { printf("Invalid string\n"); }
%%

int yywrap()
{
    return 1;
}

int main()
{
    printf("Enter a string: ");
    yylex();
    return 0;
}