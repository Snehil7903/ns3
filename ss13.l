%{
#include <stdio.h>
%}

%%
^(ibta|ibteae|a)$    {
                        printf("Valid if-then-else statement\n");
                    }

^.+$                {
                        printf("Invalid if-then-else statement\n");
                    }

%%

int yywrap()
{
    return 1;
}

int main()
{
    printf("Enter an if-then-else statement: ");
    yylex();
    return 0;
}