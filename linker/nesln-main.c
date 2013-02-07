#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tools.h"


int init_linker(void);
void destroy_linker(void);

static char* out_fname = NULL;

int init_datas(void) {
    int ret;

    ret = init_linker();
    if (!ret) {
        fprintf(stderr, "linker initalization failed...\n");
        return FALSE;
    }
    return TRUE;
}

void destroy_datas(void) {
    destroy_linker();
}

void print_usage(void) {
    printf("motonesln [option...] [.o files]\n");
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

    dprint("main...\n");

    while( (ch = getopt(argc, argv, "ho:")) != -1) {
        switch (ch) {
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

    if (argc == 1) {
        print_usage();
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
            fprintf(stderr, "link error...\n");
            goto done;
        }

    }

    ret = RT_OK;

done:


    if (need_free_out)
        free(out_fname);

    destroy_datas(); 
    return RT_ERROR;
}

const char* get_out_fname(void) {
    return out_fname;
}

