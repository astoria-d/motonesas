%{
/*
nesas.l ...
*/
#include <stdio.h>
#include "nesas.tab.h"

#define MAXLEN  1024

#if 1
#define dprint(...)    
#endif
/*
*/
%}

DOT         [\.]
COLON       [:]
DOL         [\$]
SHARP       [#]
COMMA       [\,]
QUOTE       [\"]
NL          [\n]

ALPHA       [a-zA-Z_]
DIGIT       [0-9]

IDENT       {ALPHA}({ALPHA}|{DIGIT})*
HEXNUM         {DOL}[0-9a-fA-F]*

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
                return NL;
        }

{QUOTE} { 
            int ch=0;
            int chlen=0;
            char chbuf[MAXLEN+1];
            //memset(chbuf, 0, MAXLEN+1);

            while (1) {
                ch=input();
                if (chlen >= MAXLEN) {
                    perror("quotation too long!!\n");
                    return -1;
                }
                chbuf[chlen++] = (char)ch;

                if (ch == '\"') {
                    chbuf[chlen-1] = '\0';
                    dprint("str: %s\n", chbuf);
                    break;
                }
                if (ch == '\n') {
                    perror("quote is not closed!\n");
                    return -1;
                }
            }
            yylval.str = strdup(chbuf);
#warning    strdup must be deallocated!!!!!

            return STRING;
        }

{IDENT}         { 
                    dprint("idt: %s\n", yytext); 
                    yylval.str = strdup(yytext);
#warning    strdup must be deallocated!!!!!

                    return IDENT; 
                }
{HEXNUM}        { 
                    dprint("hex: %s\n", yytext); 
                    //yylval.hex = atoi((char*)yytext+1);
                    sscanf ((char*)yytext+1, "%x", &yylval.hex );
                    return HEX; 
                }

{DOT}           { dprint("dot: %s\n", yytext); return DOT; }
{COLON}         { dprint("col: %s\n", yytext); return COLON; }
{COMMA}           { dprint("com: %s\n", yytext); return COMMA; }
{SHARP}           { dprint("shp: %s\n", yytext); return SHARP; }
{NL}              { dprint("%s", yytext); return NL; }
{WSPACE}        {}
<<EOF>>         { /*dprint("end..!\n"); */return 0;}

.               {
                    perror("unknow character!!\n");
                    return -1;
                }
%%
/*
other c code here.
*/

int lexmain(FILE* fp) {
    dprint("lexmain...\n");
    yyin = fp;
    return yylex();
}

