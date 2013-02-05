#ifndef __segment_h__
#define __segment_h__

#include <stdio.h>
#include "tools.h"

struct symmap {
    struct dlist list;
    char *symbol;
    unsigned short addr;
};


struct seginfo {
    struct dlist list;
    char* name;
    struct symmap *sym_table;
    struct symmap *unresolved_symbol;
    unsigned short current_pc;
    unsigned short segsize;

    char* out_fname;
    FILE *fp;
};

#endif /*__segment_h__*/

