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

%union {
    int hex;
    char* str;
}
%%
line_list
    :   line
    |   line_list line
    ;

line
    :   directive NL
    |   instruction NL
    |   label NL
    |   NL
    ;

directive
    :   DOT IDENT
    {
        dprint("dir: .%s\n", $<str>2);
    }
    |   DOT IDENT IDENT
    {
        dprint("dir: .%s\n", $<str>2);
    }
    |   DOT IDENT STRING
    {
        dprint("dir: .%s\n", $<str>2);
    }
    |   DOT IDENT hex_chain
    {
        dprint("dir: .%s\n", $<str>2);
    }
    ;

hex_chain
    :   HEX
    {
        dprint("hex: %04x\n", $<hex>1);
    }
    |   hex_chain COMMA HEX
    {
        dprint("hex: %04x\n", $<hex>3);
    }
    ;


instruction
    :   mnemonic
    |   mnemonic inst_param
    ;

mnemonic
    :   IDENT
    {
        dprint("mne: %s\n", $<str>1);
    }
    ;

inst_param
    :   HEX
    {
        dprint("hex: %04x\n", $<hex>1);
    }
    |   SHARP HEX
    {
        dprint("hex: #%04x\n", $<hex>2);
    }
    |   IDENT COMMA IDENT
    {
        dprint("idt: %s, s\n", $<str>1, $<str>3);
    }
    |   IDENT
    {
        dprint("idt: %s\n", $<str>1);
    }
    ;

label
    :   IDENT COLON
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

