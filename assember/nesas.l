%{
/*
nesas.l ...
*/
#include <stdio.h>
#include "nesas.tab.h"
#include "tools.h"

#define MAXLEN  1024

#if 1
#define dprint(...)    
#endif
/*
*/

static void inc_line(void);
%}

%option noyywrap

DOT         "."
COLON       ":"
DOL         "$"
SHARP       "#"
COMMA       ","
QUOTE       "\""
LPAREN      "("
RPAREN      ")"
PLUS        "+"
MINUS       "-"
NL          "\n"

ALPHA       [a-zA-Z_]
DIGIT       [0-9]

IDENT       {ALPHA}({ALPHA}|{DIGIT})*
HEXNUM      {DOL}[0-9a-fA-F]*
DECIMAL     ({PLUS}|{MINUS})?[1-9]{DIGIT}*

WSPACE      [ \t\r]+

%%
;       {
                int ch=0;
                while (1) {
                    ch=input();
                    if (ch == '\n' || ch == EOF)
                        break;
                }
                //dprint("\n");
                inc_line(); 
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
                    parser_perror("quotation too long!!\n");
                    return R_ERROR;
                }
                chbuf[chlen++] = (char)ch;

                if (ch == '\"') {
                    chbuf[chlen-1] = '\0';
                    dprint("str: %s\n", chbuf);
                    break;
                }
                if (ch == '\n') {
                    parser_perror("quote is not closed!\n");
                    return R_ERROR;
                }
            }
            yylval.str = strdup(chbuf);

            //printf("strdup: %8x\n", yylval.str);
            return STRING;
        }

{IDENT}         { 
                    dprint("idt: %s\n", yytext); 
                    yylval.str = strdup(yytext);

                    return IDENT; 
                }
{HEXNUM}        { 
                    dprint("num: %s\n", yytext); 
                    //yylval.hex = atoi((char*)yytext+1);
                    sscanf ((char*)yytext+1, "%x", &yylval.num );
                    return NUMBER; 
                }
{DECIMAL}       { 
                    dprint("num: %s\n", yytext); 
                    //yylval.hex = atoi((char*)yytext+1);
                    sscanf ((char*)yytext, "%d", &yylval.num );
                    return NUMBER; 
                }

{DOT}           { dprint("dot: %s\n", yytext); return DOT; }
{COLON}         { dprint("col: %s\n", yytext); return COLON; }
{COMMA}           { dprint("com: %s\n", yytext); return COMMA; }
{SHARP}           { dprint("shp: %s\n", yytext); return SHARP; }
{LPAREN}           { dprint("lpr: %s\n", yytext); return LPAREN; }
{RPAREN}           { dprint("rpr: %s\n", yytext); return RPAREN; }
{NL}              { dprint("%s", yytext); inc_line(); return NL; }
{WSPACE}        {}
<<EOF>>         { /*dprint("end..!\n"); */return EOF;}

.               {
                    parser_perror("unknown character!!\n");
                    return R_ERROR;
                }
%%

static void inc_line(void) {
    yyset_lineno(yyget_lineno()+1);
}

int lexmain(FILE* fp) {
    dprint("lexmain...\n");
    yyin = fp;
    return yylex();
}

