%{
/*
nesas.l ...
*/
#include <stdio.h>

#define MAXLEN  1024

/*
*/
%}

DOT         [\.]
COLON       [:]
DOL         [\$]
SHARP       [#]
COMMA       [\,]

QUOTE       [\"]
DIGIT       [0-9]
NUMBER      {DIGIT}*
WSPACE      [ \t\r]+
ALPHA       [a-zA-Z]

%%
;       {
            int ch=0;
            while (1) {
                ch=input();
                if (ch == '\n')
                    break;
            }
            dprint("\n");
        }

{QUOTE} { 
            int ch=0;
            int chlen=0;
            char chbuf[MAXLEN+1];
            while (1) {
                ch=input();
                if (chlen >= MAXLEN) {
                    perror("quotation too long!!\n");
                    return -1;
                }
                chbuf[chlen++] = (char)ch;

                if (ch == '\"') {
                    chbuf[chlen-1] = '\0';
                    dprint(chbuf);
                    break;
                }
                if (ch == '\n') {
                    perror("quote is not closed!\n");
                    return -1;
                }
            }
        }

{NUMBER}        { dprint("%s", yytext); }
{ALPHA}         { dprint("%s", yytext); }
{DOT}           { dprint("%s", yytext); }
{COLON}         { dprint("%s", yytext); }
{DOL}           { dprint("%s", yytext); }
{COMMA}           { dprint("%s", yytext); }
{SHARP}           { dprint("%s", yytext); }
\n              { dprint("%s", yytext); }

{WSPACE}        {}
.               {}
<<EOF>>         { printf("end..!\n"); return 0;}
%%
/*
other c code here.
*/

int lexmain() {
    printf("lexmain...\n");
    return yylex();
}

