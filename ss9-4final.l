%{
#include <stdio.h>
#include <string.h>

int isPalindrome(char str[])
{
    int i, j;
    int len = strlen(str);

    for(i = 0, j = len - 1; i < j; i++, j--)
    {
        if(str[i] != str[j])
            return 0;
    }

    return 1;
}
%}

%%
[ab]+ {
    if(isPalindrome(yytext))
        printf("The string %s is a Palindrome.\n", yytext);
    else
        printf("The string %s is NOT a Palindrome.\n", yytext);
}

[ \t\n]+    ;

.           printf("Invalid input! Use only 'a' and 'b'.\n");
%%

int main()
{
    printf("Enter a string over {a,b}: ");
    yylex();
    return 0;
}

int yywrap()
{
    return 1;
}