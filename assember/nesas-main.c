#include <stdio.h>
#include "tools.h"

int init_datas(void) {
    int ret;
    ret = inst_encode_init();
    if (!ret)
        return ret;
    ret = init_segment();
    if (!ret)
        return ret;

    return TRUE;
}

void destroy_datas(void) {
    inst_encode_terminate();
    destroy_segment();
}

int main (int argc, char** argv) {
    FILE* fp;
    int need_close=0;
    int ret;

    printf("main...\n");

    ret = init_datas();
    if (!ret) {
        perror("initialization failure...\n");
        return R_ERROR;
    }

    if (argc > 1) {
        fp=fopen(argv[1], "r");
        if (fp == NULL) {
            perror("invalid file!\n");
            return R_ERROR;
        }
        need_close = 1;
    }
    else
        fp=stdin;
    //lexmain(fp);
    parsermain(fp);

    if (need_close)
        fclose(fp);

    destroy_datas(); 
    return R_OK;
}

