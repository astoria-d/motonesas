#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "segment.h"
#include "obj-format.h"


#define WORK_BUF_SIZE   1024

void molf_header_create(FILE* fp, unsigned short seg_cnt) {
    struct molfhdr mh = {
        {
            '\117', 'M', 'O', 'L', 'F'
        }, 
        seg_cnt, 
        sizeof(struct molfhdr)};

    fwrite (&mh, sizeof(struct molfhdr), 1, fp);
}

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
        fwrite(seg->name, strlen(seg->name) + 1, 1, fp);
    }
    else {
        char c = '\0';
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
            fwrite(&sym->addr, 2, 1, fp);
            fwrite(sym->symbol, strlen(sym->symbol) + 1, 1, fp);

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
            fwrite(&sym->addr, 2, 1, fp);
            fwrite(sym->symbol, strlen(sym->symbol) + 1, 1, fp);

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
        if (fseek(fp, hdr_strt, SEEK_SET)) {
            fseek(fp, back_pos, SEEK_SET);
            return FALSE;
        }

        fread(&segh, sizeof(short) * 3, 1, fp);

        segh.seg_name = malloc(WORK_BUF_SIZE);
        j = 0;
        while(fread(&ch, 1, 1, fp) > 0) {
            if (j >= WORK_BUF_SIZE)
                segh.seg_name = realloc(segh.seg_name, WORK_BUF_SIZE);
            segh.seg_name[j++] = ch;
            if (ch == '\0') {
                break;
            }
        }

        if (seg->name && !strcmp(segh.seg_name, seg->name)) {
            seg_found = TRUE;
        }
        else if ( seg->name == NULL && segh.seg_name[0] == '\0') {
            seg_found = TRUE;
        }

        if (seg_found) {
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




