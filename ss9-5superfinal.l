%{
#include <stdio.h>
#include <string.h>

int isPalindrome(char str[])
{
    int i, j;
    int len = strlen(str);

    /* c must be exactly in the middle */
    if (len % 2 == 0)
        return 0;

    if (str[len / 2] != 'c')
        return 0;

    /* Check W and reverse(W) */
    for (i = 0, j = len - 1; i < j; i++, j--)
    {
        if (str[i] != str[j])
            return 0;
    }

    return 1;
}
%}

%%
[abc]+ {
    if (isPalindrome(yytext))
        printf("The string %s is of the form WcW^R.\n", yytext);
    else
        printf("The string %s is NOT of the form WcW^R.\n", yytext);
}

[ \t\n]+ ;

. {
    printf("Invalid input! Use only a, b and c.\n");
}
%%

int main()
{
    printf("Enter a string: ");
    yylex();
    return 0;
}

int yywrap()
{
    return 1;
}