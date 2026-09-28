%{
#include "y.tab.h"
%}

%%

[a]     { return 'a'; }
[b]     { return 'b'; }
\n      { return '\n'; }
.       { return yytext[0]; }

%%

int yywrap()
{
    return 1;
}