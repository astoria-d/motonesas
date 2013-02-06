#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "obj-format.h"
#include "segment.h"

struct seginfo * segh2segi (struct seghdr *sgh, FILE* objfile, const char* fname) {
    struct seginfo * pseg = malloc (sizeof(struct seginfo));

    dlist_init (&pseg->list);
    pseg->name = strdup(sgh->seg_name);
    dprint("segment: %s\n", pseg->name);

    return pseg;
}

int load_object (const char* obj_fname) {
    FILE* fp;
    struct molfhdr* molh;
    int segh_start;
    int i, seg_cnt;

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

        //pseg = segh2segi(sgh, fp, obj_fname);
        ///must make list of seg!!!
        //
        //segh_start += sgh->seg_data_size;

        free(sgh);
    }

    return TRUE;
}
