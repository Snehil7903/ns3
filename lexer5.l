%{
#include <stdio.h>
%}

%%
^(((a|b)*(c|d)*)*|ab*c*d)$      { printf("Matched Expression (a): %s\n", yytext); }
^((0|1)*|0*1*)$                  { printf("Matched Expression (b): %s\n", yytext); }
^(01*2|0*2|1)+$                  { printf("Matched Expression (c): %s\n", yytext); }
\n                               { /* ignore newline */ }
.                                { printf("Not Matched: %s\n", yytext); }
%%

int yywrap() {
    return 1;
}

int main() {
    printf("Enter a string to test against expressions (a, b, c):\n");
    yylex();
    return 0;
}
