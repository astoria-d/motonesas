#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tools.h"


static char* out_fname = NULL;

int init_datas(void) {
    int ret;

    return TRUE;
}

void destroy_datas(void) {
}

void print_usage(void) {
    printf("motonesln [option...] [.o files]\n");
    printf("Options:\n");
    printf("\t-h: print this page.\n");
    printf("\t-o [output]: output object file.\n");
}

int main (int argc, char** argv) {
    FILE* fp;
    int ret;
    int ch;
    extern int optind;
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
    }
    else {
        print_usage();
        return RT_ERROR;
    }

    if (out_fname == NULL) {
        const char* in_fname = argv[1];
        char *p, *pp;

        need_free_out = TRUE;
        out_fname = (char*) malloc ( strlen(in_fname) + 4 + 1);
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
        strcpy(pp, ".nes");
    }
    dprint("outfile: %s\n", out_fname);

    if (ret != 0) {
        fprintf(stderr, "link error...\n");
        goto done;
    }

done:

    fclose(fp);

    if (need_free_out)
        free(out_fname);

    destroy_datas(); 
    return RT_OK;
}

const char* get_out_fname(void) {
    return out_fname;
}

