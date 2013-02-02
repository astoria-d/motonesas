#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

void print_usage(void) {
    printf("motonesas [option...] [asmfile]\n");
    printf("Options:\n");
    printf("\t-h: print this page.\n");
    printf("\t-o [output]: output object file.\n");
}

int main (int argc, char** argv) {
    FILE* fp;
    int need_close=0;
    int ret;
    int ch;
    extern int optind;
    char* out_fname = NULL;
    int need_free_out = FALSE;

    dprint("main...\n");

    while( (ch = getopt(argc, argv, "ho:")) != -1) {
        switch (ch) {
            case 'o':
                out_fname = optarg;
                break;
            case 'h':
            default:
                print_usage();
                return 0;
        }
    }
    argc -= optind - 1;
    argv += optind - 1;


    ret = init_datas();
    if (!ret) {
        fprintf(stderr, "initialization failure...\n");
        return RT_ERROR;
    }

    if (argc > 1) {
        const char* fname = argv[1];
        fp=fopen(fname, "r");
        if (fp == NULL) {
            fprintf(stderr, "invalid file [");
            fprintf(stderr, fname);
            fprintf(stderr, "]\n");
            return RT_ERROR;
        }
        need_close = 1;
    }
    else
        fp=stdin;

    if (out_fname == NULL) {
        const char* in_fname = argv[1];
        char *p, *pp;

        if (fp == stdin) {
            print_usage();
           goto done;
        }

        need_free_out = TRUE;
        out_fname = (char*) malloc ( strlen(in_fname) + 2 + 1);
        strcpy(out_fname, in_fname);

        //search for fname extention.
        p = out_fname;
        pp = NULL;
        while ( *p != '\0' ) {
            if (*p == '.') {
                pp = p;
            }
            p++;
        }

        if (pp == NULL) {
            pp = p + strlen(p) - 1;
        }
        ///
        strcpy(pp, ".o");
    }
    dprint("outfile: %s\n", out_fname);

    //lexmain(fp);
    parsermain(fp);

done:

    if (need_close)
        fclose(fp);

    if (need_free_out)
        free(out_fname);

    destroy_datas(); 
    return RT_OK;
}

