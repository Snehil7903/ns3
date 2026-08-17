%{
#include <iostream>
using namespace std;

#define INT       1
#define CHAR      2
#define VOID      3
#define IF        4
#define ELSE      5
#define WHILE     6
#define RETURN    7
#define INTEGER   8
#define REAL      9
#define PTR_OP    10
#define AND_OP    11
#define OR_OP     12
#define DOT_OP    13
%}

%%

"int"       { cout << yytext << " -> INT" << endl; }
"char"      { cout << yytext << " -> CHAR" << endl; }
"void"      { cout << yytext << " -> VOID" << endl; }
"if"        { cout << yytext << " -> IF" << endl; }
"else"      { cout << yytext << " -> ELSE" << endl; }
"while"     { cout << yytext << " -> WHILE" << endl; }
"return"    { cout << yytext << " -> RETURN" << endl; }

[+-]?[0-9]+\.[0-9]+([Ee][+-]?[0-9]+)? {
                cout << yytext << " -> REAL" << endl;
            }

[+-]?[0-9]+([Ee][+-]?[0-9]+) {
                cout << yytext << " -> REAL" << endl;
            }

[+-]?[0-9]+ {
                cout << yytext << " -> INTEGER" << endl;
            }

">"         { cout << yytext << " -> PTR_OP" << endl; }
"&&"        { cout << yytext << " -> AND_OP" << endl; }
"||"        { cout << yytext << " -> OR_OP" << endl; }
"."         { cout << yytext << " -> DOT_OP" << endl; }

[;:+*/]     { cout << yytext << " -> " << yytext << endl; }

[{}(),=&~\[\]] {
                cout << yytext << " -> " << yytext << endl;
            }

[ \t\n]+    { /* Ignore whitespace */ }

.           {
                cout << yytext << " -> UNKNOWN" << endl;
            }

%%

int main()
{
    cout << "Enter input:" << endl;
    yylex();
    return 0;
}