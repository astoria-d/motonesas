#include <stdio.h>
#include "tools.h"
#include "obj-format.h"

int load_object (const char* obj_fname) {
    FILE* fp;
    struct molfhdr* molh;
    int segh_start;
    int seg_cnt;

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

    return TRUE;
}
