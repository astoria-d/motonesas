
#include <stdlib.h>
#include "tools.h"
#include "segment.h"
#include "symtool.h"

void clear_symtbl(struct symmap *psym) {
    //dprint("free symbol %s.\n", psym->symbol);
    free(psym->symbol);
    free(psym);
}

void clear_symtbl_list(struct symmap* psym) {

    while (psym != NULL) {
        struct symmap* pp = psym;
        psym = (struct symmap*) psym->list.next;
        dlist_remove((struct dlist*)pp);
        clear_symtbl(pp);
    } 
}

