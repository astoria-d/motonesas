%{
/*
nesas.l ...
*/
#include <stdio.h>

static int commentout=0;

/*
*/
%}

DIGIT       [0-9]
NUMBER      {DIGIT}*
WSPACE      [ \t\r]+
ALPHA       [a-zA-Z]

%%
;               {commentout=1; /*dprint("semi!!!");*/ }
{NUMBER}         { if (!commentout) dprint("%s", yytext); }
{ALPHA}         { if (!commentout) dprint("%s", yytext); }
\n              {commentout=0; dprint("\n"); }
{WSPACE}        {}
.               {}
<<EOF>>         { printf("end..!\n"); return 0;}
%%
/*
other c code here.
*/

int lexmain() {
    commentout=0; 
    printf("lexmain...\n");
    return yylex();
}

