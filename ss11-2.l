%{
#include "parser.tab.h"
%}

%%

[a-zA-Z_][a-zA-Z0-9_]*   { return ID; }

[ \t\r]+                 { /* Ignore spaces and tabs */ }

\n                       { return 0; }

.                        { return yytext[0]; }

%%