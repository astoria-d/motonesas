#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "obj-format.h"
#include "segment.h"

#define DEFAULT_SEGMENT     "<default>"

static struct seginfo* seg_list;

int lookup_lmap(const char* segname, unsigned short *start, unsigned short *size);

static void clear_seglist(struct seginfo* sg_head) {
    struct seginfo* pseg;

    struct closed_file {
        struct slist l;
        FILE* fp;
    } * closed_list = NULL;
    struct closed_file *wk_list;

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

        dprint("free segmeng %s: %s\n", pp->out_fname, pp->name);
        if (pp->name)
            free(pp->name);

        //clean up files.
        //must close only one time for the fp!!
        if (!closed_list) {
            closed_list = malloc (sizeof(struct closed_file));
            closed_list->l.next = NULL;
            closed_list->fp = pp->fp;
            //dprint("file close %s\n", pp->out_fname);
            fclose(pp->fp);
        }
        else {
            struct closed_file *cl = closed_list;
            int closed = FALSE;
            while (cl != NULL) {
                if (pp->fp == cl->fp) {
                    closed = TRUE;
                    break;
                }
                cl = (struct closed_file*)cl->l.next;
            }
            if (!closed) {
                cl = malloc (sizeof(struct closed_file));
                cl->l.next = NULL;
                cl->fp = pp->fp;
                slist_add_tail(&closed_list->l, &cl->l);
                //dprint("file close %s\n", pp->out_fname);
                fclose(pp->fp);
            }
        }
        free(pp->out_fname);

        free(pp);
    } 

    //clean up closed list.
    wk_list = closed_list;
    while (wk_list != NULL) {
        struct closed_file *pp = wk_list;
        wk_list = (struct closed_file*)wk_list->l.next;
        free(pp);
    }
}


static struct seginfo * segh2segi (struct seghdr *sgh, FILE* objfile, const char* fname) {
    struct seginfo * pseg = malloc (sizeof(struct seginfo));
    unsigned short start, size;
    int ret;

    dlist_init (&pseg->list);

    if (sgh->seg_name[0] == '\0')
        pseg->name = strdup(DEFAULT_SEGMENT);
    else
        pseg->name = strdup(sgh->seg_name);

    dprint("segment: %s\n", pseg->name);
    //mv symtable
    pseg->sym_table = sgh->symbols;
    sgh->symbols = NULL;
    pseg->unresolved_symbol = sgh->unresolved_symbols;
    sgh->unresolved_symbols = NULL;

    ///current pc is set to segment start address.
    ret = lookup_lmap(pseg->name, &start, &size);
    if (!ret) {
        fprintf(stderr, "invalid segment [%s]\n", pseg->name);
        free (pseg->name);
        free (pseg);
        return NULL;
    }
    pseg->current_pc = start;
    pseg->segsize = sgh->seg_data_size;

    pseg->out_fname = strdup(fname);
    pseg->fp = objfile;

    return pseg;
}

static int add_segment_list(struct seginfo *pseg) {
    if (seg_list == NULL)
        seg_list  = pseg;
    else 
    {
        int ret;
        unsigned short start, size;
        struct seginfo* node;

        ret = lookup_lmap(pseg->name, &start, &size);
        if (!ret) {
            fprintf(stderr, "invalid segment [%s]\n", pseg->name);
            return FALSE;
        }

        node = seg_list;
        while(node != NULL) {
            //now current_pc is pointing at the segment start address.
            if (node->current_pc > start) {
                break;
            }
            node = (struct seginfo*) node->list.next;
        }

        //sort segment by the start address
        if (node)
            dlist_add_prev(&node->list, &pseg->list);
        else
            dlist_add_tail(&seg_list->list, &pseg->list);
    }
    return TRUE;
}

int load_object (const char* obj_fname) {
    FILE* fp;
    struct molfhdr* molh;
    int segh_start;
    int i, seg_cnt;
    int ret;

    fp=fopen(obj_fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "invalid file [%s]\n", obj_fname);
        return FALSE;
    }

    //load MOLF file header.
    molh = load_mh(fp);
    if (!molh) {
        fclose(fp);
        return FALSE;
    }
    seg_cnt = molh->seg_cnt;
    segh_start = molh->segh_off;
    free(molh);


    fseek(fp, segh_start, SEEK_SET);
    for (i = 0; i < seg_cnt; i++) {
        struct seghdr *sgh;
        struct seginfo *pseg;

        //fseek(fp, segh_start, SEEK_SET);

        sgh = load_segh(fp);

        if (!sgh) {
            free(sgh);
            fclose(fp);
            return FALSE;
        }

        pseg = segh2segi(sgh, fp, obj_fname);
        if (!pseg) {
            clear_segh(sgh);
            return FALSE;
        }
        ret = add_segment_list(pseg);
        if (!ret) {
            clear_segh(sgh);
            return FALSE;
        }
        
        //segh_start += sgh->seg_data_size;

        clear_segh(sgh);
    }

    return TRUE;
}

int link_segment(const char* out_name) {
    return TRUE;
}

void destroy_linker(void) {
    if (seg_list)
        clear_seglist (seg_list);
}

int init_linker(void) {
    seg_list = NULL;
    return TRUE;
}

