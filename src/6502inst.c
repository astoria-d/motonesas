
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tools.h"

struct instmap {
    char mnemonic[4];
    unsigned char adr_mode;
    unsigned char opcode;
};

#define NUM_ALPHA 'Z' - 'A' + 1
#define ALPHA_INDEX(ch) ch - 'A'

/* 6502 instructions
 * adr_mode
 * 0:Zero Page
 * 1:Zero Page, X
 * 2:Zero Page, Y
 * 3:Absolute
 * 4:Absolute, X
 * 5:Absolute, Y
 * 6:Indirect
 * 7:Implied
 * 8:Accumulator
 * 9:Immediate
 * 10:Relative
 * 11:(Indirect, X)
 * 12:(Indirect), Y
 * */
static struct instmap instructions[] = {
#include "opcode"
};

struct inst_node {
    struct slist * next;
    struct instmap * inst;
};

static struct inst_node * inst_srch [ NUM_ALPHA ] ;

int inst_tbl_init(void) {
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
    return 1;
}

void inst_tbl_free(void) {
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
        if (!strcasecmp(mnemonic, p->inst->mnemonic))
            return 1;

        p = (struct inst_node*) p->next;
    } 

    return 0;
}


