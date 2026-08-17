%{
#include <stdio.h>

/* Define token values/names for display purposes */
#define INT 257
#define CHAR 258
#define VOID 259
#define IF 260
#define ELSE 261
#define WHILE 262
#define RETURN 263
#define NUM 264
#define REAL 265
#define PTR_OP 266
#define AND_OP 267
#define OR_OP 268
#define DOT_OP 269
%}

DIGIT       [0-9]
INT_NUM     [+-]?{DIGIT}+
EXPONENT    [eE][+-]?{DIGIT}+
REAL_NUM    [+-]?({DIGIT}+\.{DIGIT}*|\.{DIGIT}+){EXPONENT}?|[+-]?{DIGIT}+{EXPONENT}

%%

"int"       { printf("Token: INT (%s)\n", yytext); return INT; }
"char"      { printf("Token: CHAR (%s)\n", yytext); return CHAR; }
"void"      { printf("Token: VOID (%s)\n", yytext); return VOID; }
"if"        { printf("Token: IF (%s)\n", yytext); return IF; }
"else"      { printf("Token: ELSE (%s)\n", yytext); return ELSE; }
"while"     { printf("Token: WHILE (%s)\n", yytext); return WHILE; }
"return"    { printf("Token: RETURN (%s)\n", yytext); return RETURN; }

"->"        { printf("Token: PTR_OP (%s)\n", yytext); return PTR_OP; }
"&&"        { printf("Token: AND_OP (%s)\n", yytext); return AND_OP; }
"||"        { printf("Token: OR_OP (%s)\n", yytext); return OR_OP; }
"."         { printf("Token: DOT_OP (%s)\n", yytext); return DOT_OP; }

{REAL_NUM}  { printf("Token: REAL (%s)\n", yytext); return REAL; }
{INT_NUM}   { printf("Token: INT_NUM (%s)\n", yytext); return NUM; }

"-"|"+"|"*"|"/" { printf("Token: Operator '%c'\n", yytext[0]); return yytext[0]; }

";"|"{"|"}"|","|"="|")"|"("|"&"|"~"|"["|"]" { printf("Token: Separator '%c'\n", yytext[0]); return yytext[0]; }

[ \t\n]+    { /* Ignore whitespace and newlines */ }
.           { printf("Unknown character: %s\n", yytext); }

%%

int yywrap() {
    return 1;
}

int main() {
    printf("Enter input code for Lexical Analysis:\n\n");
    yylex();
    return 0;
}