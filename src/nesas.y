%{
/*
nesas.y ...
*/
#include <stdio.h>

extern FILE *yyin;
/*
*/
%}

%token  DOT COLON SHARP COMMA NL
%token  IDENT HEX STRING

%%
line_list
    :   line
    |   line_list line
    ;

line
    :   instruction NL
    |   directive NL
    |   label NL
    |   NL
    ;

label
    :   IDENT COLON
    ;

instruction
    :   mnemonic
    |   mnemonic params
    ;

directive
    :   DOT IDENT
    |   DOT IDENT STRING
    |   DOT IDENT IDENT
    |   DOT IDENT params
    ;

mnemonic
    :   IDENT
    ;

params
    :   param
    |   params COMMA param
    ;

param
    :   HEX
    |   SHARP HEX
    |   IDENT
    |   STRING
    ;
%%

int parsermain(FILE* fp) {
    printf("parsermain...\n");
    yyin = fp;
    return yyparse();
}

int yyerror(const char* s) {
    printf(s);
    printf("\n");
    return -1;
}

