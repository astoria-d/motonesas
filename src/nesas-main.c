#include <stdio.h>

int init_datas(void) {
    int ret;
    ret = inst_tbl_init();
    if (!ret)
        return ret;

    return 1;
}

void destroy_datas(void) {
    inst_tbl_free();
}

int main (int argc, char** argv) {
    FILE* fp;
    int need_close=0;
    int ret;

    printf("main...\n");

    ret = init_datas();
    if (!ret) {
        perror("initialization failure...\n");
        return -1;
    }

    if (argc > 1) {
        fp=fopen(argv[1], "r");
        if (fp == NULL) {
            perror("invalid file!\n");
            return -1;
        }
        need_close = 1;
    }
    else
        fp=stdin;
    //lexmain(fp);
    parsermain(fp);

    if (need_close)
        fclose(fp);
    return 0;
}

