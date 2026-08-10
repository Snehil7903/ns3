%{
#include <stdio.h>
%}

LETTER      [a-zA-Z]
DIGIT       [0-9]
SPECIAL     [!@#$%&*_]

%%

{SPECIAL}({LETTER}|{DIGIT})*     { printf("Token1: %s\n", yytext); }

{DIGIT}({LETTER}|{SPECIAL})*     { printf("Token2: %s\n", yytext); }

{LETTER}({DIGIT}|{SPECIAL})*{LETTER}   { printf("Token3: %s\n", yytext); }

[\t\n ]+    ;

.           { printf("Invalid: %s\n", yytext); }

%%

int main() {
    printf("enter input:\n");
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}