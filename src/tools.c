
#include <stdio.h>
#include "tools.h"

void dprint(char* s, char* arg1, unsigned int arg2, unsigned int arg3, 
        unsigned int arg4, unsigned int arg5, unsigned int arg6) {
     printf(s, arg1, arg2, arg3, arg4, arg5, arg6 );
    /*
     **/
}

void list_init (struct slist* top) {
    top->next = NULL;
}

void slist_add_tail (struct slist* dest, struct slist* node) {
    struct slist *p = dest;
    struct slist *pp = dest;
    while( (p = p->next) != NULL ) {
        pp = p;
    }
    pp->next = node;
}

