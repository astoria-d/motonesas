
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "tools.h"

struct symmap {
    struct dlist list;
    char *symbol;
    unsigned short addr;
};


struct seginfo {
    struct dlist list;
    char* name;
    struct symmap *sym_table;
    struct symmap *unresolved_symbol;
    unsigned short current_pc;

    char* out_fname;
    FILE *fp;
};

/*
 * these three must be removed.
 * */

static struct seginfo *segment_list;
static struct seginfo *current_seg;

unsigned short get_current_pc(void);
struct seginfo * get_current_seginfo(void);
const char* get_out_fname(void);

int add_symbol (const char* symbol) {
    struct symmap *psym, *pp;
    struct seginfo *pseg = get_current_seginfo();

    psym = pseg->sym_table;
    if (psym == NULL) {
        psym = malloc(sizeof (struct symmap));
        dlist_init(&psym->list);
        psym->symbol = strdup(symbol);
        psym->addr = get_current_pc();
        pseg->sym_table = psym;

        dprint("%s:\n", symbol);
        return TRUE;
    }
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

static void clear_symtbl(struct symmap *psym) {
    //dprint("free symbol %s.\n", psym->symbol);
    free(psym->symbol);
    free(psym);
}

static void clear_symtbl_list(struct symmap* psym) {

    while (psym != NULL) {
        struct symmap* pp = psym;
        psym = (struct symmap*) psym->list.next;
        dlist_remove((struct dlist*)pp);
        clear_symtbl(pp);
    } 
}

static void add_unresolved_lookup(void) {
    dprint("unresolvvd!!!\n");
#warning need to add unresolved symbol handling...
}


int addr_lookup(const char* symbol, unsigned short* return_addr) {
    struct symmap* psym;
    int found = FALSE;

    psym = get_current_seginfo()->sym_table;
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

static struct seginfo * lookup_segment(const char* segname) {
    struct seginfo *pseg = NULL;

    pseg = segment_list;
    while ( pseg != NULL) {
        if (pseg->name == NULL && segname == NULL) {
            return pseg;
        }
        if (pseg->name == NULL && segname != NULL) {
            pseg = (struct seginfo*)pseg->list.next;
            continue;
        }
        else if (!strcmp(pseg->name, segname)) {
            return pseg;
        }
        pseg = (struct seginfo*)pseg->list.next;
    }
    return NULL;
}

static struct seginfo *new_segment (const char* segname) {
    struct seginfo *seg;
    const char* out_name;
    int fname_len;

    seg = malloc(sizeof (struct seginfo));
    dlist_init(&seg->list);
    if (segname)
        seg->name = strdup(segname);
    else
        seg->name = NULL;
    seg->current_pc = 0;

    if (segment_list == NULL) {
        segment_list = seg; 
    }
    else {
        dlist_add_tail((struct dlist*)segment_list, (struct dlist*)seg);
    }

    //temporary segment file name <outname>.seg<segname>
    out_name = get_out_fname();
    if (segname) {
        fname_len = strlen (out_name) + strlen (segname) + 4;
        seg->out_fname = malloc (fname_len + 1);
        sprintf(seg->out_fname, "%s.seg%s", out_name, segname);
    }
    else {
        fname_len = strlen (out_name) + 4;
        seg->out_fname = malloc (fname_len + 1);
        sprintf(seg->out_fname, "%s.seg", out_name);
    }

    seg->fp = fopen(seg->out_fname, "w+");
    //dprint("new_segment fp:%08x\n", seg->fp);
    if (seg->fp == NULL) {
        if (seg->name)
            free(seg->name);
        free(seg->out_fname);
        free(seg);
        return NULL;
    }

    return seg;
}

struct seginfo * set_segment(const char* segname) {
    struct seginfo *seg;

    dprint("segment %s:\n", segname);
    seg = lookup_segment(segname);
    if (seg == NULL) {
        seg = new_segment(segname);
    }
    current_seg = seg;

    return seg;
}

struct seginfo * get_current_seginfo(void) {
    if (current_seg == NULL) {
        set_segment(NULL);
    }
    return current_seg;
}

unsigned short get_current_symtbl(void) {
}

unsigned short get_current_pc(void) {
    return get_current_seginfo()->current_pc;
}

void clear_seglist(void) {
    struct seginfo* pseg;

    dprint("clear_seglist.\n");
    pseg = segment_list;
    while (pseg != NULL) {
        struct seginfo* pp = pseg;
        pseg = (struct seginfo*) pseg->list.next;

        dlist_remove((struct dlist*)pp);
        //must be clear resolve tbl.
        if (pp->sym_table)
            clear_symtbl_list(pp->sym_table);

        //dprint("free segmeng %s\n", pp->name);
        if (pp->name)
            free(pp->name);

        //clean up files.
        fclose(pp->fp);
        remove (pp->out_fname);
        free(pp->out_fname);

        free(pp);
    } 
}

void move_current_pc(short offset) {
    get_current_seginfo()->current_pc += offset;
}

FILE * get_current_file(void) {
    //dprint("get_current_file fp:%08x\n", get_current_seginfo()->fp);
    return get_current_seginfo()->fp;
}

int finalize_segment(void) {
    struct seginfo* pseg;
    FILE* out;

    out = fopen(get_out_fname(), "w");
    if (out == NULL)
        return FALSE;

    pseg = segment_list;
    while (pseg != NULL) {
        FILE* fp = pseg->fp;
        fpos_t pos;
        char* buf;
        int size;

        dprint ("segment file: %s\n", pseg->out_fname);
        fgetpos(fp, &pos);
        size = pos.__pos;
        buf = malloc(size);
        if (buf == NULL) {
            fclose(out);
            return FALSE;
        }
        
        //move to top position
        rewind(fp);
        fread(buf, size, 1, fp);
        fwrite(buf, size, 1, out);

        pseg = (struct seginfo*) pseg->list.next;
    }
    fclose(out);
    return TRUE;
}

int init_segment(void) {
    segment_list = current_seg = NULL; 
    return TRUE;
}

void destroy_segment(void) {
    clear_seglist(); 
}


