
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "tools.h"
#include "segment.h"
#include "symtool.h"

#if 0
#define dprint(...)    
#endif

/*
 * these three must be removed.
 * */

static struct seginfo *segment_list;
static struct seginfo *current_seg;

unsigned short get_current_pc(void);
struct seginfo * get_current_seginfo(void);
const char* get_out_fname(void);
void molf_header_create(FILE* fp, unsigned short seg_cnt);
void seg_header_create(FILE* fp, struct seginfo* seg);
int set_seg_header_pos(FILE* fp, struct seginfo* seg, unsigned short start);
int get_abs_addr(unsigned short abs_addr);

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
        pseg->unresolved_symbol = NULL;

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

void add_unresolved_ref(const char* symbol) {
    struct seginfo *pseg = get_current_seginfo();
    struct symmap *unres_sym;

    dprint("unresolved symbol: %s\n", symbol);
    unres_sym = malloc(sizeof (struct symmap));
    dlist_init(&unres_sym->list);
    unres_sym->symbol = strdup(symbol);
    //current_pc points to the current instruction.
    //referece is next to the pc
    unres_sym->addr = pseg->current_pc + 1;

    if (pseg->unresolved_symbol == NULL) {
        pseg->unresolved_symbol = unres_sym;
    }
    else {
        dlist_add_tail(pseg->unresolved_symbol, &unres_sym->list);
    }

}


int addr_lookup(const char* symbol, unsigned short* return_addr) {
    struct symmap* psym;
    int found = FALSE;

    psym = get_current_seginfo()->sym_table;
    while (psym != NULL) {
        struct symmap* pp = psym;

        if (!strcmp(symbol, psym->symbol)) {
            /*
            dprint("addr_lookup %s found %04x, current:%08x\n", psym->symbol, 
                    psym->addr, get_current_pc());
            */

            found = TRUE;
            *return_addr = psym->addr;
            break;
        }
        psym = (struct symmap*) psym->list.next;
    } 
    return found;
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

unsigned short get_current_pc(void) {
    return get_current_seginfo()->current_pc;
}

static void clear_seglist(struct seginfo* sg_head) {
    struct seginfo* pseg;

    //dprint("clear_seglist.\n");
    pseg = sg_head;
    while (pseg != NULL) {
        struct seginfo* pp = pseg;
        pseg = (struct seginfo*) pseg->list.next;

        dlist_remove((struct dlist*)pp);
        //clear symbol table
        if (pp->sym_table)
            clear_symtbl_list(pp->sym_table);

        //clear unresolved symbol table.
        if (pp->unresolved_symbol)
            clear_symtbl_list(pp->unresolved_symbol);

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
    FILE* obj_file;
    int seg_cnt;

    ///resolve unresolved symbol first.
    pseg = segment_list;
    while (pseg != NULL) {
        FILE* fp = pseg->fp;

        //dprint ("segment file: %s\n", pseg->out_fname);

        //resolve unresolved symbol in the segment.
        current_seg = pseg;
        if (pseg->unresolved_symbol) {
            struct symmap *unres = pseg->unresolved_symbol;
            unsigned short addr;
            int ret;
            do {
                struct symmap *free_sym = unres;

                ret = addr_lookup(unres->symbol, &addr);
                if (ret) {
                    if (addr - unres->addr > 0xFF) {
                        //address offset is too far!
                        return FALSE;
                    }
                    addr = get_abs_addr(addr);
                    /*
                    dprint("symbol ref at %04x to %s resolved, %04x.\n", 
                            unres->addr, unres->symbol, addr);
                    */
                    fseek(fp, unres->addr, SEEK_SET);
                    fwrite(&addr, 2, 1, fp);
                }
                unres = (struct symmap*) unres->list.next;

                if (ret) {
                    //remove symbol from unresolved list.
                    ret = dlist_remove(&free_sym->list);
                    if (!ret) {
                        //if the free_sym is the last node, then clean up the list top.
                        pseg->unresolved_symbol = NULL;
                    }
                    clear_symtbl(free_sym);
                }
            } while (unres);
        }
        
        pseg = (struct seginfo*) pseg->list.next;
    }


    obj_file = fopen(get_out_fname(), "w+");
    if (obj_file == NULL)
        return FALSE;

    //object file creation.
    seg_cnt = dlist_count(&segment_list->list);
    molf_header_create(obj_file, seg_cnt);

    //write segment header
    pseg = segment_list;
    while (pseg != NULL) {
        seg_header_create(obj_file, pseg);
        pseg = (struct seginfo*) pseg->list.next;
    }

    //copy setment file.
    pseg = segment_list;
    while (pseg != NULL) {
        FILE* fp = pseg->fp;
        char* buf;
        long pos;
        int size;

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);

        buf = malloc(size);
        if (buf == NULL) {
            fclose(obj_file);
            return FALSE;
        }

        //move to top position
        rewind(fp);
        //get current pos in obj_file file.
        pos = ftell(obj_file);

        //copy xxxx.o.segXXX file to xxx.o file.
        fread(buf, size, 1, fp);
        fwrite(buf, size, 1, obj_file);
        pseg->segsize = size;

        //set header position.
        if (!set_seg_header_pos(obj_file, pseg, pos)) {
            fclose(obj_file);
            return FALSE;
        }

        pseg = (struct seginfo*) pseg->list.next;
    }

    fclose(obj_file);

    return TRUE;
}

int init_segment(void) {
    segment_list = current_seg = NULL; 
    return TRUE;
}

void destroy_segment(void) {
    clear_seglist(segment_list); 
}


