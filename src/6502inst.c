
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "tools.h"
#include "6502inst.h"

#define NUM_SPACE 15

struct instmap {
    char mnemonic[4];
    unsigned char adr_mode;
    unsigned char opcode;
};

#define NUM_ALPHA 'Z' - 'A' + 1
#define ALPHA_INDEX(ch) ch - 'A'

/* 
 * 6502 instructions
 * adressing mode        instruction length
 * 0:Zero Page           2
 * 1:Zero Page, X        2
 * 2:Zero Page, Y        2
 * 3:Absolute            3
 * 4:Absolute, X         3
 * 5:Absolute, Y         3
 * 6:Indirect            3
 * 7:Implied             1
 * 8:Accumulator         1
 * 9:Immediate           2
 * 10:Relative           2
 * 11:(Indirect, X)      2
 * 12:(Indirect), Y      2
 * 
 * */

/*
 * instructions by alphabetical order
 * */
static struct instmap instructions[] = {
#include "opcode"
};

struct inst_node {
    struct slist * next;
    struct instmap * inst;
};

/*
 * instruction definition search table
 * */
static struct inst_node * inst_srch [ NUM_ALPHA ] ;


struct symmap {
    struct dlist list;
    char *symbol;
    unsigned short addr;
};

static struct symmap *symbol_tbl;
static struct symmap *unresolved_symbol;

static unsigned short current_pc;

static int inst_tbl_init(void) {
    int index = 0;
    const int num_inst = sizeof (instructions) / sizeof( struct instmap);

    dprint ("inst_tbl_init.\n");

    memset (&inst_srch, sizeof (inst_srch), 0);

    while (index < num_inst) {
        struct inst_node * node = malloc ( sizeof (struct inst_node) );
        char inst_ch;
        int ii;
        node->next = NULL;
        node->inst = &instructions[index];

        inst_ch = toupper(node->inst->mnemonic[0]);
        ii = ALPHA_INDEX(inst_ch);

        if ( inst_srch [ii] == NULL ) {
            inst_srch [ii] = node;
        }
        else {
            slist_add_tail((struct slist*)inst_srch[ii], (struct slist*)node);
        }
        index++;
    }
    return TRUE;
}

static void inst_tbl_free(void) {
    dprint ("inst_tbl_free.\n");

    int index = 0;

    while (index < NUM_ALPHA) {
        if ( inst_srch [index] != NULL ) {
            ///free the data chain...
            struct inst_node * p= inst_srch[index];
            struct inst_node * pp= inst_srch[index];
            do {
                pp = p;
                p = (struct inst_node*) pp->next;
                free(pp);
            } while (p != NULL);
        }
        index++;
    }
}

int check_inst(const char* mnemonic) {
    char inst_ch = toupper(*mnemonic);
    int ii = ALPHA_INDEX(inst_ch);

    struct inst_node * p= inst_srch[ii];
    while (p != NULL) {
        if (!strcasecmp(mnemonic, p->inst->mnemonic)) {
            return TRUE;
        }

        p = (struct inst_node*) p->next;
    } 

    return FALSE;
}

int add_symbol (const char* symbol) {
    struct symmap* psym, *pp;

    if (symbol_tbl == NULL) {
        symbol_tbl = malloc(sizeof (struct symmap));
        dlist_init(&symbol_tbl->list);
        symbol_tbl->symbol = strdup(symbol);
        symbol_tbl->addr = current_pc;

        dprint("%s:\n", symbol);
        return TRUE;
    }
    psym = symbol_tbl;
    do {
        if (!strcmp(psym->symbol, symbol)) {
            return FALSE;
        }
        pp = psym;
        psym = (struct symmap*) psym->list.next;
    } while (psym != NULL);

    /*add new symbol..*/
    psym = malloc(sizeof (struct symmap));
    dlist_init(&psym->list);
    psym->symbol = strdup(symbol);
    psym->addr = current_pc;
    dlist_add_next((struct dlist*)pp, (struct dlist*)psym);

    dprint("%s:\n", symbol);

    return TRUE;

}

void clear_symtbl(void) {
    struct symmap* psym;

    psym = symbol_tbl;
    while (psym != NULL) {
        struct symmap* pp = psym;
        psym = (struct symmap*) psym->list.next;

        dlist_remove((struct dlist*)pp);
        free(pp->symbol);
        free(pp);
    } 
}

static void add_unresolved_lookup(void) {
    dprint("unresolvvd!!!\n");
#warning need to add unresolved symbol handling...
}


int addr_lookup(const char* symbol, unsigned short* return_addr) {
    struct symmap* psym;
    int found = FALSE;

    psym = symbol_tbl;
    while (psym != NULL) {
        struct symmap* pp = psym;

        if (!strcmp(symbol, psym->symbol)) {
            found = TRUE;
            *return_addr = psym->addr;
            break;
        }
        psym = (struct symmap*) psym->list.next;
    } 
    if (!found) {
        add_unresolved_lookup();
    }
    return found;
}

int get_rel_addr(unsigned short abs_addr) {
    return (int) abs_addr - current_pc ;
}

int resolve_sym(void) {
}

/*
 * returns the length of opcode.
 * returns 0 if invalid.
 * */
static int encode_inst(const char* mnemonic, int addr_mode, int num, 
        unsigned char* out_bytes /*output array*/ ) {

    char inst_ch = toupper(*mnemonic);
    int ii = ALPHA_INDEX(inst_ch);
    int found = FALSE;
    int len = 0;
    int operand_size = 0;

    struct inst_node * p= inst_srch[ii];

    //operand size check. 0 ~ 0xFF or -0x80 ~ 0x7F
    if ( num > 0xFFFF || num < -0x80 ) {
        //operand too big.
        return 0;
    }

    //search mnemonic
    while (p != NULL) {
        if (!strcasecmp(mnemonic, p->inst->mnemonic)) {
            found = TRUE;
            break;
        }
        p = (struct inst_node*) p->next;
    } 

    if (!found) 
        return 0;


    //search address mode
    found = FALSE;
    if (addr_mode == PARAM_NON) {
        while (p != NULL) {
            if (strcasecmp(mnemonic, p->inst->mnemonic)) {
                break;
            }
            if (p->inst->adr_mode == 7 || p->inst->adr_mode == 8) {
                len = 1;
                *out_bytes = p->inst->opcode;
                return len;
            }
            p = (struct inst_node*) p->next;
        } 
    }

    if ( num <= 0xFF ) 
        operand_size = 1;
    else
        operand_size = 2;

    //indirect always takes two byte param
    if (addr_mode == (PARAM_NUM | PARAM_INDIR))
        operand_size = 2;

    //jum/jsr always take two byte param
    if (!strcasecmp(mnemonic, "JMP") || !strcasecmp(mnemonic, "JSR")) 
        operand_size = 2;

    if (operand_size == 1) {
        while (p != NULL) {
            if (strcasecmp(mnemonic, p->inst->mnemonic)) {
                break;
            }
            if (addr_mode == PARAM_IMMED && p->inst->adr_mode == 9) {
                found = TRUE;
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDEX_X) && p->inst->adr_mode == 1) {
                //zero page, x
                found = TRUE;
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDEX_Y) && p->inst->adr_mode == 2) {
                //zero page, y 
                found = TRUE;
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDEX_INDIR) && p->inst->adr_mode == 11) {
                //index indirect (index, x)
                found = TRUE;
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDIR_INDEX) && p->inst->adr_mode == 12) {
                //indirect index (index), y
                found = TRUE;
                break;
            }
            else if (addr_mode == PARAM_NUM && 
                    ( p->inst->adr_mode == 0 || p->inst->adr_mode == 10 ) ) {
                //zero page, or relative.
                found = TRUE;
                break;
            }
            p = (struct inst_node*) p->next;
        } 
        if (found) {
            len = 2;
            *out_bytes++ = p->inst->opcode;
            *out_bytes = num;
            return len;
        }
    }
    else {
        while (p != NULL) {
            if (strcasecmp(mnemonic, p->inst->mnemonic)) {
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDEX_X) && p->inst->adr_mode == 4) {
                //absolute, x
                found = TRUE;
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDEX_Y) && p->inst->adr_mode == 5) {
                //absolute, y 
                found = TRUE;
                break;
            }
            else if (addr_mode == (PARAM_NUM | PARAM_INDIR) && p->inst->adr_mode == 6) {
                //indirect index (index), y
                found = TRUE;
                break;
            }
            else if (addr_mode == PARAM_NUM && 
                    ( p->inst->adr_mode == 3 || p->inst->adr_mode == 3 ) ) {
                //absolute or indirect
                found = TRUE;
                break;
            }
            p = (struct inst_node*) p->next;
        } 
        if (found) {
            len = 3;
            //byte order: little endian.
            *out_bytes++ = p->inst->opcode;
            *out_bytes++ = 0xFF & num;
            *out_bytes = 0xFF & (num >> 8);
            return len;
        }
    }

    return 0;
}

#define DEB_PRINT

static void deb_print_inst(const char* mnemonic, int addr_mode, int num) {
#ifdef DEB_PRINT
    int len = 0;
    int space = 0;
    unsigned char opcode[3];
    int i;

    len = encode_inst(mnemonic, addr_mode, num, opcode);
    if (len == 0) 
        return;

    if (addr_mode == PARAM_NON) {
        printf("%04x:  %s", current_pc, mnemonic);
    }
    if (addr_mode == PARAM_IMMED) {
        printf("%04x:  %s #$%02x", current_pc, mnemonic, num);
        space = 5;
    }
    else if (addr_mode & PARAM_NUM ) {
        if (len == 2) {
            num &= 0xff;
            if (addr_mode & PARAM_INDEX_X) {
                printf("%04x:  %s $%02x, X", current_pc, mnemonic, num);
                space = 7;
            }
            else if (addr_mode & PARAM_INDEX_Y) {
                printf("%04x:  %s $%02x, Y", current_pc, mnemonic, num);
                space = 7;
            }
            else if (addr_mode & PARAM_INDEX_INDIR) { 
                printf("%04x:  %s ($%02x, Y)", current_pc, mnemonic, num);
                space = 9;
            }
            else if (addr_mode & PARAM_INDIR_INDEX) { 
                printf("%04x:  %s ($%02x), X", current_pc, mnemonic, num);
                space = 9;
            }
            else {
                printf("%04x:  %s $%02x", current_pc, mnemonic, num);
                space = 4;
            }
        }
        else {
            num &= 0xffff;
            if (addr_mode & PARAM_INDEX_X) {
                printf("%04x:  %s $%04x, X", current_pc, mnemonic, num);
                space = 9;
            }
            else if (addr_mode & PARAM_INDEX_Y) { 
                printf("%04x:  %s $%04x, Y", current_pc, mnemonic, num);
                space = 9;
            }
            else if (addr_mode & PARAM_INDIR) { 
                printf("%04x:  %s ($%04x)", current_pc, mnemonic, num);
                space = 8;
            }
            else if (addr_mode & PARAM_INDEX_INDIR) { 
                printf("%04x:  %s ($%04x, Y)", current_pc, mnemonic, num);
                space = 9;
            }
            else if (addr_mode & PARAM_INDIR_INDEX) { 
                printf("%04x:  %s ($%04x), X", current_pc, mnemonic, num);
                space = 9;
            }
            else {
                printf("%04x:  %s $%04x", current_pc, mnemonic, num);
                space = 6;
            }
        }
    }

    for (i = 0; i < NUM_SPACE - space; i++)
        printf(" ");
    for (i = 0; i < len; i++)
        printf("%02x ", opcode[i]);
    printf("\n");
#endif /*DEB_PRINT*/
}

int write_inst(const char* mnemonic, int addr_mode, int num) {
    int len;
    char opcode[3];
    len = encode_inst(mnemonic, addr_mode, num, opcode);
    if (len == 0) 
        return FALSE;

    deb_print_inst(mnemonic, addr_mode, num);
    if (addr_mode == PARAM_NON) {
        current_pc += 1;
    }
    if (addr_mode == PARAM_IMMED) {
        current_pc += 2;
    }
    else if (addr_mode & PARAM_NUM) {
        current_pc += 2;
    }

    return TRUE;
}

static void deb_print_str(const char* str) {
#ifdef DEB_PRINT
    int i, len = strlen (str);
    for (i = 0; i < len; i++) {
        printf("%c ", *(str + i));
    }
    printf ("00\n");
#endif /*DEB_PRINT*/
}

void write_str(const char* str) {
    deb_print_str(str);
}

static void deb_print_word(int num) {
#ifdef DEB_PRINT
    printf("%02x ", 0xFF & num);
    printf("%02x ", 0xFF & (num >> 8));
#endif /*DEB_PRINT*/
}

void write_word_data(int num) {
    deb_print_word(num);
}

static void deb_print_byte(int num) {
#ifdef DEB_PRINT
    printf("%02x ", 0xFF & num);
#endif /*DEB_PRINT*/
}

void write_byte_data(int num) {
    deb_print_byte(num);
}

void deb_print_nl(void) {
#ifdef DEB_PRINT
    printf("\n");
#endif /*DEB_PRINT*/
}

int inst_encode_init() {
    inst_tbl_init(); 
    symbol_tbl = NULL; 
    unresolved_symbol = NULL; 
    current_pc = 0; 
}

void inst_encode_terminate() {
    inst_tbl_free();
    clear_symtbl(); 
}

