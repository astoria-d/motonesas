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

ALPHA       [a-zA-Z_]
DIGIT       [0-9]

IDENT       {ALPHA}({ALPHA}|{DIGIT})*
HEXDIGIT         [0-9a-fA-F]*

WSPACE      [ \t\r]+

%%
;       {
                int ch=0;
                while (1) {
                    ch=input();
                    if (ch == '\n')
                        break;
                }
                //dprint("\n");
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
                    //dprint(chbuf);
                    printf("str: %s\n", chbuf);
                    break;
                }
                if (ch == '\n') {
                    perror("quote is not closed!\n");
                    return -1;
                }
            }
        }

{IDENT}         { dprint("idt: %s\n", yytext); }
{HEXDIGIT}        { dprint("hex: %s\n", yytext); }
{DOT}           { dprint("dot: %s\n", yytext); }
{COLON}         { dprint("col: %s\n", yytext); }
{DOL}           { dprint("dol: %s\n", yytext); }
{COMMA}           { dprint("com: %s\n", yytext); }
{SHARP}           { dprint("shp: %s\n", yytext); }
\n              { dprint("%s", yytext); }

{WSPACE}        {}
.               {}
<<EOF>>         { /*printf("end..!\n"); */return 0;}
%%
/*
other c code here.
*/

int lexmain(FILE* fp) {
    printf("lexmain...\n");
    yyin = fp;
    return yylex();
}

