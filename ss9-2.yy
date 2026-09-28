%{
#include <iostream>
#include <string>
using namespace std;

int yylex();
void yyerror(const char *s);

string input;
bool palindrome = true;
int leftIndex = 0;
%}

%token A B

%%

start:
    palindrome '\n'
    {
        if (palindrome)
            cout << "Palindrome" << endl;
        else
            cout << "Not a Palindrome" << endl;
    }
    ;

palindrome:
      /* empty */
    | 'a' palindrome 'a'
    | 'b' palindrome 'b'
    | 'c'
    ;

%%

void yyerror(const char *s)
{
    cout << "Invalid input" << endl;
}

int main()
{
    cout << "Enter a string: ";
    yyparse();
    return 0;
}