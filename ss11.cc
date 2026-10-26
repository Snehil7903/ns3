%{
#include <iostream>
using namespace std;

int yylex();
void yyerror(const char *s);
%}

/* Declare tokens */
%token ID

/* Operator precedence and associativity */
%left '+' '-'
%left '*' '/'
%right '^'
%right UMINUS

%%

input:
      expression
      {
          cout << "Valid arithmetic expression!" << endl;
      }
    ;

expression:
      expression '+' expression
    | expression '-' expression
    | expression '*' expression
    | expression '/' expression
    | expression '^' expression
    | '(' expression ')'
    | '-' expression %prec UMINUS
    | ID
    ;

%%

void yyerror(const char *s)
{
    cout << "Invalid arithmetic expression!" << endl;
}

int main()
{
    cout << "Enter an arithmetic expression: ";
    yyparse();
    return 0;
}