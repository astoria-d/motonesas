#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "segment.h"
#include "obj-format.h"


#define WORK_BUF_SIZE   1024

void store_string(FILE* fp, const char* str) {
    short len;

    len = strlen (str);
    fwrite(&len, 2, 1, fp);
    while (len-- > 0)
        fwrite(str++, 1, 1, fp);
    fwrite(str++, 1, 1, fp);
}

char* load_string(FILE* fp) {
    short len;
    char* str;
    int i = 0;
    fread(&len, 2, 1, fp);
    str = malloc (len + 1);
    while (len-- > 0)
        fread(str + i++, 1, 1, fp);
    fread(str + i++, 1, 1, fp);
    return str;
}

void molf_header_create(FILE* fp, unsigned short seg_cnt) {
    struct molfhdr mh = {
        {
            '\117', 'M', 'O', 'L', 'F'
        }, 
        seg_cnt, 
        sizeof(struct molfhdr)};

    fwrite (&mh, 1, sizeof(struct molfhdr), fp);
}

/*
 * write segh data (except for unresolved symbol list)
 *
 * */
void seg_header_create(FILE* fp, struct seginfo* seg) {
    long header_start, back_pos;
    short tmp;
    struct symmap *sym;


    header_start = ftell(fp);

    //segh_size is filled later.
    tmp = 0;
    fwrite(&tmp, 2, 1, fp);
    //seg_start_pos is filled later.
    tmp = -1;
    fwrite(&tmp, 2, 1, fp);
    //seg_data_size is filled later.
    tmp = 0;
    fwrite(&tmp, 2, 1, fp);

    //seg_name
    if (seg->name) {
        store_string(fp, seg->name);
    }
    else {
        short l = 0;
        char c = '\0';
        fwrite(&l, 2, 1, fp);
        fwrite(&c, 1, 1, fp);
    }
    //dprint("segmeng: %s\n", seg->name);

    //symbol
    sym = seg->sym_table;
    if (sym) {
        //symbol cnt
        tmp = dlist_count(&sym->list);
        fwrite(&tmp, 2, 1, fp);

        while (sym) {
            store_string(fp, sym->symbol);
            fwrite(&sym->addr, 2, 1, fp);

            sym = (struct symmap*) sym->list.next;
        }
    }
    else {
        tmp = 0;
        fwrite(&tmp, 2, 1, fp);
    }

    //unresolved symbol
    sym = seg->unresolved_symbol;
    if (sym) {
        //symbol cnt
        tmp = dlist_count(&sym->list);
        fwrite(&tmp, 2, 1, fp);

        while (sym) {
            store_string(fp, sym->symbol);
            fwrite(&sym->addr, 2, 1, fp);

            sym = (struct symmap*) sym->list.next;
        }
    }
    else {
        tmp = 0;
        fwrite(&tmp, 2, 1, fp);
    }

    //fill segh_size.
    back_pos = ftell(fp); 
    fseek(fp, header_start, SEEK_SET);
    tmp = back_pos - header_start;
    fwrite(&tmp, 2, 1, fp);

    //dprint("segh size: %d\n", tmp);

    //back to old pos.
    fseek(fp, back_pos, SEEK_SET);
    
}

/*
 * complete sgment header info
 *
 * */
int set_seg_header_pos(FILE* fp, struct seginfo* seg, unsigned short start) {
    long back_pos;
    struct molfhdr molh; 
    struct seghdr segh; 
    unsigned short hdr_strt;
    int i, segcnt;

    back_pos = ftell(fp);

    rewind(fp);
    fread(&molh, sizeof(struct molfhdr), 1, fp);
    if (!MOL_CHK((&molh))) {
        fseek(fp, back_pos, SEEK_SET);
        return FALSE;
    }
    
    hdr_strt = molh.segh_off;
    segcnt = molh.seg_cnt;

    for (i = 0; i < segcnt; i++) {
        char ch;
        int j;
        int seg_found = FALSE;
        short tmp;

        ///search for the segment.
        if (fseek(fp, hdr_strt, SEEK_SET)) {
            fseek(fp, back_pos, SEEK_SET);
            return FALSE;
        }

        fread(&tmp, 2 , 1, fp);
        segh.segh_size = tmp;
        fread(&tmp, 2 , 1, fp);
        segh.seg_start_pos = tmp;
        fread(&tmp, 2 , 1, fp);
        segh.seg_data_size = tmp;

        segh.seg_name = load_string(fp);

        if (seg->name && !strcmp(segh.seg_name, seg->name)) {
            seg_found = TRUE;
        }
        else if ( seg->name == NULL && segh.seg_name[0] == '\0') {
            seg_found = TRUE;
        }

        if (seg_found) {
            ///write the segment info..
            //
            fseek(fp, hdr_strt + 2, SEEK_SET);
            //fill seg_start_pos
            fwrite(&start, 2, 1, fp);
            //fill seg_data_size
            fwrite(&seg->segsize, 2, 1, fp);

            free(segh.seg_name);

            //back to old pos.
            fseek(fp, back_pos, SEEK_SET);
            return TRUE;
        }
        free(segh.seg_name);
        hdr_strt += segh.segh_size;
    }

    fseek(fp, back_pos, SEEK_SET);
    return FALSE;
}

struct molfhdr * load_mh(FILE* fp) {
    int len;
    struct molfhdr* ret = malloc(sizeof(struct molfhdr));

    if (ret == NULL)
        return NULL;

    len = fread(ret, 1, sizeof(struct molfhdr), fp);
    if (len != sizeof(struct molfhdr)) {
        free (ret);
        return NULL;
    }

    //magic number check
    if (!MOL_CHK(ret)) {
        free (ret);
        return FALSE;
    }

    return ret;
}


struct seghdr* load_segh(FILE* fp) {
    struct seghdr* sgh;
    short segh_size;
    int len;
    short tmp;

    sgh = malloc(sizeof(struct seghdr));
    if (!sgh)
        return NULL;

    fread(&tmp, 2, 1, fp);
    sgh->segh_size = tmp;

    fread(&tmp, 2, 1, fp);
    sgh->seg_start_pos = tmp;

    fread(&tmp, 2, 1, fp);
    sgh->seg_data_size = tmp;

    //get seg_name
    sgh->seg_name = load_string(fp);
    if (!sgh->seg_name) {
        free(sgh);
        return NULL;
    }
    
    dprint("segment: %s\n", sgh->seg_name);

    //get sym cnt
    fread(&tmp, 2, 1, fp);
    sgh->symbol_cnt = tmp;
    //get sym entry
    if (sgh->symbol_cnt == 0) {
        sgh->symbols = NULL;
    }
    else {
        int i;
        sgh->symbols = NULL;
        for (i = 0; i < sgh->symbol_cnt; i++) {
            struct symmap *sm;

            sm = malloc(sizeof(struct symmap));
            dlist_init(&sm->list);

            sm->symbol = load_string(fp);
            fread(&tmp, 2, 1, fp);
            sm->addr = tmp;

            if (sgh->symbols == NULL)
                sgh->symbols = sm;
            else
                dlist_add_tail(sgh->symbols, &sm->list);
            dprint("  symbol: %s\n", sm->symbol);
        }
    }

    //get unresolved sym cnt
    fread(&tmp, 2, 1, fp);
    sgh->unresolve_cnt = tmp;
    //get unres sym entry
    if (sgh->unresolve_cnt == 0) {
        sgh->unresolved_symbols = NULL;
    }
    else {
        int i;
        sgh->unresolved_symbols = NULL;
        for (i = 0; i < sgh->unresolve_cnt; i++) {
            struct symmap *sm;

            sm = malloc(sizeof(struct symmap));
            dlist_init(&sm->list);

            sm->symbol = load_string(fp);
            fread(&tmp, 2, 1, fp);
            sm->addr = tmp;

            if (sgh->unresolved_symbols == NULL)
                sgh->unresolved_symbols = sm;
            else
                dlist_add_tail(sgh->unresolved_symbols, &sm->list);
            dprint("  unresolved: %s\n", sm->symbol);
        }
    }

    return sgh;
}

