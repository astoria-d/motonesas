#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tools.h"


int init_linker(void);
void destroy_linker(void);

int sort_segment(void);
int link_segment(FILE* outf);
int init_lmap(void);
void destory_lmap(void);

int init_datas(void) {
    int ret;

    ret = init_linker();
    if (!ret) {
        fprintf(stderr, "linker initalization failed...\n");
        return FALSE;
    }
    ret = init_lmap();
    if (!ret) {
        fprintf(stderr, "linker config init failed...\n");
        return FALSE;
    }
    return TRUE;
}

void destroy_datas(void) {
    destroy_linker();
    destory_lmap();
}

void print_usage(void) {
    printf("motonesln -l linkmap [option...] [.o files]\n");
    printf("Options:\n");
    printf("\t-h: print this page.\n");
    printf("\t-o [output]: output object file.\n");
}

int main (int argc, char** argv) {
    int ret;
    int ch;
    extern int optind;
    int need_free_out = FALSE;
    int i;
    char* out_fname = NULL;
    char* lmap_fname = NULL;
    FILE *outfp;

    dprint("main...\n");

    while( (ch = getopt(argc, argv, "ho:l:")) != -1) {
        switch (ch) {
            case 'l':
                lmap_fname = optarg;
                break;
            case 'o':
                out_fname = optarg;
                break;
            case 'h':
            default:
                print_usage();
                return RT_ERROR;
        }
    }
    argc -= optind - 1;
    argv += optind - 1;


    ret = init_datas();
    if (!ret) {
        fprintf(stderr, "initialization failure...\n");
        return RT_ERROR;
    }

    if (argc == 1 || lmap_fname == NULL) {
        print_usage();
        return RT_ERROR;
    }

    ret = load_lmap(lmap_fname);
    if (!ret) {
        fprintf(stderr, "link map load error...\n");
        return RT_ERROR;
    }

    if (out_fname == NULL && argc == 2) {
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
    else if (argc > 2) {
        out_fname = "out.nes";
    }
    dprint("outfile: %s\n", out_fname);

    for (i = 1; i < argc; i++) {
        const char* fname = argv[i];
        dprint("input file: %s\n", fname);

        ret = load_object(fname);
        if (!ret) {
            fprintf(stderr, "load object error...\n");
            goto done;
        }
    }

    //linker main work here.
    outfp = fopen (out_fname, "w");
    if (outfp == NULL) {
        fprintf(stderr, "out file error...\n", out_fname);
        goto done;
    }
    ret = link_segment(outfp);
    if (!ret) {
        fclose(outfp);
        fprintf(stderr, "link error...\n");
        goto done;
    }

    dprint("link succeeded\n");
    ret = RT_OK;
    fclose(outfp);

done:


    if (need_free_out)
        free(out_fname);

    destroy_datas(); 
    return RT_ERROR;
}

