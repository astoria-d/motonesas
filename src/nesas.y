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
        dprint("dir: .%s %s\n", $<str>2, $<str>3);
    }
    |   DOT IDENT STRING
    {
        dprint("dir: .%s \"%s\"\n", $<str>2, $<str>3);
    }
    |   DOT IDENT {
        dprint("dir: .%s ", $<str>2);
    } hex_chain {
        dprint("\n");
    }
    ;

hex_chain
    :   HEX
    {
        dprint("%04x", $<hex>1);
    }
    |   hex_chain COMMA HEX
    {
        dprint(", %04x", $<hex>3);
    }
    ;


instruction
    :   IDENT {
        dprint("mne: %s\n", $<str>1);
        if (!check_inst($<str>1)) {
            perror("invalid instruction\n");
            return -1;
        }
    }
    |   IDENT {
        dprint("mne: %s ", $<str>1);
        if (!check_inst($<str>1)) {
            perror("invalid instruction\n");
            return -1;
        }
    } inst_param
    ;

inst_param
    :   HEX
    {
        dprint("%04x\n", $<hex>1);
    }
    |   SHARP HEX
    {
        dprint("#%04x\n", $<hex>2);
    }
    |   IDENT COMMA IDENT
    {
        dprint("%s, %s\n", $<str>1, $<str>3);
    }
    |   IDENT
    {
        dprint("%s\n", $<str>1);
    }
    ;

label
    :   IDENT COLON
    {
        dprint("lbl: %s\n", $<str>1);
    }
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

