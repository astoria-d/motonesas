
#include <stdio.h>
#include "tools.h"

void dprint(char* s, char* arg1, unsigned int arg2, unsigned int arg3, 
        unsigned int arg4, unsigned int arg5, unsigned int arg6) {
     printf(s, arg1, arg2, arg3, arg4, arg5, arg6 );
    /*
     **/
}

void slist_add_tail (struct slist* dest, struct slist* node) {
    struct slist *p = dest;
    struct slist *pp = dest;
    while( (p = p->next) != NULL ) {
        pp = p;
    }
    pp->next = node;
}

void dlist_init (struct dlist* node) {
    node->next = node->prev = NULL;
}

void dlist_add_next (struct dlist* dest, struct dlist* node) {
    struct dlist* next_node = dest->next;

    if (next_node) {
        next_node->prev = node;
    }
    dest->next = node;
    node->prev = dest;
    node->next = next_node;
}

void dlist_remove (struct dlist* node) {
    struct dlist* next_node = node->next;
    struct dlist* prev_node = node->prev;

    if (next_node) {
        next_node->prev = prev_node;
    }
    if (prev_node) {
        prev_node->next = next_node;
    }
}


