%{
#include <stdio.h>
#include <string.h>

char input[100];
int pos = 0;

int S();

int C()
{
    if(input[pos] == 'b')
    {
        pos++;
        return 1;
    }
    return 0;
}

int S()
{
    if(input[pos] == 'a')
    {
        pos++;
        return 1;
    }

    if(input[pos] == 'i')
    {
        pos++;

        if(!C())
            return 0;

        if(input[pos] != 't')
            return 0;

        pos++;

        if(!S())
            return 0;

        if(input[pos] == 'e')
        {
            pos++;

            if(!S())
                return 0;
        }

        return 1;
    }

    return 0;
}
%}

%%
[^\n]+ {
    strcpy(input, yytext);
    pos = 0;

    if(S() && input[pos] == '\0')
        printf("Valid if-then-else statement\n");
    else
        printf("Invalid if-then-else statement\n");
}

\n ;
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