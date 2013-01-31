%{
/*
nesas.y ...
*/
//#define YYERROR_VERBOSE
//#define YYDEBUG

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

%token  DOT COLON SHARP COMMA LPAREN RPAREN NL
%token  IDENT NUMBER STRING

%union {
    int num;
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
            parser_perror("invalid directive\n");
            YYERROR;
        }
    }
    |   DOT IDENT IDENT
    {
        dprint("dir2: .%s %s\n", $<str>2, $<str>3);
        if (!directive_check($<str>2, DIR_PARAM_IDENT, $<str>3)) {
            parser_perror("invalid directive\n");
            YYERROR;
        }
    }
    |   DOT IDENT STRING
    {
        dprint("dir: .%s \"%s\"\n", $<str>2, $<str>3);
        if (!directive_check($<str>2, DIR_PARAM_LITERAL, $<str>3)) {
            parser_perror("invalid directive\n");
            YYERROR;
        }
    }
    |   DOT IDENT {
        dprint("dir: .%s ", $<str>2);
    } num_chain {
        dprint("\n");
    }
    ;

num_chain
    :   NUMBER
    {
        dprint("%04x", $<num>1);
    }
    |   num_chain COMMA NUMBER 
    {
        dprint(", %04x", $<num>3);
    }
    ;


instruction
    :   IDENT {
        dprint("mne: %s\n", $<str>1);
        if (!check_inst($<str>1)) {
            parser_perror("invalid instruction\n");
            YYERROR;
        }
        cur_inst = $<str>1;
        write_inst(NULL, cur_inst, PARAM_NON, 0);
    }
    |   IDENT {
        dprint("mne: %s ", $<str>1);
        if (!check_inst($<str>1)) {
            parser_perror("invalid instruction\n");
            YYERROR;
        }
        cur_inst = $<str>1;
    } inst_param
    ;

inst_param
    :   NUMBER
    {
        dprint("%04x\n", $<num>1);
        write_inst(NULL, cur_inst, PARAM_NUM, $<num>1);
    }
    |   SHARP NUMBER
    {
        dprint("#%04x\n", $<num>2);
        write_inst(NULL, cur_inst, PARAM_IMMED, $<num>2);
    }
    |   NUMBER COMMA IDENT
    {
        char ch;
        int param;

        //second parameter is either X or Y.
        if (strcasecmp($<str>3, "X") && strcasecmp($<str>3, "Y")) {
            parser_perror("invalid parameter\n");
            YYERROR;
        }
        dprint("%04x, %s\n", $<num>1, $<str>3);

        param = PARAM_NUM;
        ch = toupper(*$<str>3);
        param |= (ch == 'X' ? PARAM_INDEX_X : PARAM_INDEX_Y);
        write_inst(NULL, cur_inst, param, $<num>1);
    }
    |   IDENT COMMA IDENT
    {
        char ch;
        int param;
        short addr;

        //second parameter is either X or Y.
        if (strcasecmp($<str>3, "X") && strcasecmp($<str>3, "Y")) {
            parser_perror("invalid parameter\n");
            YYERROR;
        }
        dprint("%s, %s\n", $<str>1, $<str>3);

        param = PARAM_NUM;
        ch = toupper(*$<str>3);
        param |= (ch == 'X' ? PARAM_INDEX_X : PARAM_INDEX_Y);
        addr = addr_lookup($<str>1);
        write_inst(NULL, cur_inst, param, addr);
    }
    |   LPAREN NUMBER RPAREN
    {
        dprint("%04x\n", $<num>2);

        write_inst(NULL, cur_inst, PARAM_NUM | PARAM_INDIR, $<num>2);
    }
    |   LPAREN NUMBER COMMA IDENT RPAREN
    {
        //second parameter is X.
        if (strcasecmp($<str>4, "X")) {
            parser_perror("invalid parameter\n");
            YYERROR;
        }
        dprint("(%04x, %s)\n", $<num>2, $<str>4);

        write_inst(NULL, cur_inst, PARAM_NUM | PARAM_INDEX_INDIR, $<num>2);
    }
    |   LPAREN NUMBER RPAREN COMMA IDENT
    {
        //second parameter is Y.
        if (strcasecmp($<str>5, "Y")) {
            parser_perror("invalid parameter\n");
            YYERROR;
        }
        dprint("(%04x), %s\n", $<num>2, $<str>5);

        write_inst(NULL, cur_inst, PARAM_NUM | PARAM_INDIR_INDEX, $<num>2);
    }
    |   IDENT
    {
        dprint("%s\n", $<str>1);
        short addr;
        addr = addr_lookup($<str>1);
        write_inst(NULL, cur_inst, PARAM_NUM, get_rel_addr(addr));
    }
    ;

label
    :   IDENT COLON
    {
        dprint("lbl: %s\n", $<str>1);
        if (!add_symbol($<str>1)) {
            parser_perror("invalid symbol\n");
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
    parser_perror(s);
    //return value is discarded..
    return R_OK;
}

