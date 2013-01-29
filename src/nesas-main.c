#include <stdio.h>

int main (int argc, char** argv) {
    FILE* fp;

    printf("main...\n");

    int need_close=0;
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

