
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "tools.h"
#include "6502inst.h"

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


int addr_lookup(const char* symbol) {
    struct symmap* psym;
    int found = FALSE;
    unsigned short addr = 0;

    psym = symbol_tbl;
    while (psym != NULL) {
        struct symmap* pp = psym;

        if (!strcmp(symbol, psym->symbol)) {
            found = TRUE;
            addr = psym->addr;
            break;
        }
        psym = (struct symmap*) psym->list.next;
    } 
    if (!found) {
        add_unresolved_lookup();
    }
    return addr;
}

short get_rel_addr(unsigned short abs_addr) {
    return (int) abs_addr - current_pc ;
}

int resolve_sym(void) {
}

static void deb_print_inst(const char* mnemonic, int addr_mode, unsigned short num) {
    if (addr_mode == PARAM_NON) {
        printf("%04x:  %s\n", current_pc, mnemonic);
    }
    if (addr_mode == PARAM_IMMED) {
        printf("%04x:  %s, #$%02x\n", current_pc, mnemonic, num);
    }
    else if (addr_mode & PARAM_NUM ) {
        if (addr_mode & PARAM_INDEX_X) 
            printf("%04x:  %s, $%04x, X\n", current_pc, mnemonic, num);
        else if (addr_mode & PARAM_INDEX_Y) 
            printf("%04x:  %s, $%04x, Y\n", current_pc, mnemonic, num);
        else if (addr_mode & PARAM_INDIR) 
            printf("%04x:  %s, ($%04x)\n", current_pc, mnemonic, num);
        else if (addr_mode & PARAM_INDEX_INDIR) 
            printf("%04x:  %s, ($%04x, Y)\n", current_pc, mnemonic, num);
        else if (addr_mode & PARAM_INDIR_INDEX) 
            printf("%04x:  %s, ($%04x), X\n", current_pc, mnemonic, num);
        else 
            printf("%04x:  %s, $%04x\n", current_pc, mnemonic, num);
    }
}

int write_inst(FILE* fp, const char* mnemonic, int addr_mode, unsigned short num) {
    fp = stdout;
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

