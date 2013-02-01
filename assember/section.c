
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tools.h"

struct symmap {
    struct dlist list;
    char *symbol;
    unsigned short addr;
};

static struct symmap *symbol_tbl;
static struct symmap *unresolved_symbol;


int add_symbol (const char* symbol) {
    struct symmap* psym, *pp;

    if (symbol_tbl == NULL) {
        symbol_tbl = malloc(sizeof (struct symmap));
        dlist_init(&symbol_tbl->list);
        symbol_tbl->symbol = strdup(symbol);
        symbol_tbl->addr = get_current_pc();

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
    psym->addr = get_current_pc();
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

int resolve_sym(void) {
}

int init_section(void) {
    symbol_tbl = NULL; 
    unresolved_symbol = NULL; 
    return TRUE;
}

void destroy_section(void) {
    clear_symtbl(); 
}


