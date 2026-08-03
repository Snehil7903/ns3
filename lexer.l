%{
#include <iostream>
#include <cstring>
using namespace std;
%}

/* Definitions */
letter      [a-zA-Z_]
digit       [0-9]
id          {letter}({letter}|{digit})*

%%

"/*"([^*]*"*"+)*"/"        { cout << "Comment detected\n"; }

"begin"    { cout << "Keyword: begin\n"; }
"end"      { cout << "Keyword: end\n"; }
"if"       { cout << "Keyword: if\n"; }
"then"     { cout << "Keyword: then\n"; }
"else"     { cout << "Keyword: else\n"; }
"for"      { cout << "Keyword: for\n"; }
"do"       { cout << "Keyword: do\n"; }
"while"    { cout << "Keyword: while\n"; }
"switch"   { cout << "Keyword: switch\n"; }
"case"     { cout << "Keyword: case\n"; }
"default"  { cout << "Keyword: default\n"; }
"break"    { cout << "Keyword: break\n"; }
"continue" { cout << "Keyword: continue\n"; }
"goto"     { cout << "Keyword: goto\n"; }

{id}       { cout << "Identifier: " << yytext << endl; }

{digit}+   { cout << "Number: " << yytext << endl; }

[ \t\n]+   ;   // Ignore spaces

.          { cout << "Unknown: " << yytext << endl; }

%%

int main()
{
    cout << "Enter input:\n";
    yylex();
    return 0;
}

int yywrap()
{
    return 1;
}