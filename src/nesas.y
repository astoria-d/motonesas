%{
/*
nesas.y ...
*/
#include <stdio.h>
#include "tools.h"
#include "6502inst.h"
#include "6502directive.h"

extern FILE *yyin;
const char* cur_inst;


#if 1
#define dprint(...)    
#endif
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
        if (!directive_check($<str>2, DIR_PARAM_NON, NULL)) {
            perror("invalid directive\n");
            YYERROR;
        }
    }
    |   DOT IDENT IDENT
    {
        dprint("dir2: .%s %s\n", $<str>2, $<str>3);
        if (!directive_check($<str>2, DIR_PARAM_IDENT, $<str>3)) {
            perror("invalid directive\n");
            YYERROR;
        }
    }
    |   DOT IDENT STRING
    {
        dprint("dir: .%s \"%s\"\n", $<str>2, $<str>3);
        if (!directive_check($<str>2, DIR_PARAM_LITERAL, $<str>3)) {
            perror("invalid directive\n");
            YYERROR;
        }
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
            YYERROR;
        }
        cur_inst = $<str>1;
        write_inst(NULL, cur_inst, PARAM_NON, 0);
    }
    |   IDENT {
        dprint("mne: %s ", $<str>1);
        if (!check_inst($<str>1)) {
            perror("invalid instruction\n");
            YYERROR;
        }
        cur_inst = $<str>1;
    } inst_param
    ;

inst_param
    :   HEX
    {
        dprint("%04x\n", $<hex>1);
        write_inst(NULL, cur_inst, PARAM_HEX, $<hex>1);
    }
    |   SHARP HEX
    {
        dprint("#%04x\n", $<hex>2);
        write_inst(NULL, cur_inst, PARAM_IMMED, $<hex>2);
    }
    |   IDENT COMMA IDENT
    {
        char ch;
        int param;

        //second parameter is either X or Y.
        if (strcasecmp($<str>3, "X") && strcasecmp($<str>3, "Y")) {
            perror("invalid parameter\n");
            YYERROR;
        }
        dprint("%s, %s\n", $<str>1, $<str>3);

        param = PARAM_HEX;
        ch = toupper($<str>3);
        param |= ch == 'X' ? PARAM_INDEX_X : PARAM_INDEX_Y;
        write_inst(NULL, cur_inst, param, $<hex>2);
    }
    |   IDENT
    {
        dprint("%s\n", $<str>1);
        short addr;
        addr = addr_lookup($<str>1);
        write_inst(NULL, cur_inst, PARAM_HEX, get_rel_addr(addr));
    }
    ;

label
    :   IDENT COLON
    {
        dprint("lbl: %s\n", $<str>1);
        if (!add_symbol($<str>1)) {
            perror("invalid symbol\n");
            YYERROR;
        }
    }
    ;

%%

int parsermain(FILE* fp) {
    printf("parsermain...\n");
    cur_inst = NULL;
    yyin = fp;
    return yyparse();
}

int yyerror(const char* s) {
    printf(s);
    printf("\n");
    //return value is discarded..
    return R_OK;
}

