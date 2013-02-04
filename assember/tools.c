
#include <stdio.h>
#include "tools.h"

void dprint(char* s, char* arg1, unsigned int arg2, unsigned int arg3, 
        unsigned int arg4, unsigned int arg5, unsigned int arg6) {
     printf(s, arg1, arg2, arg3, arg4, arg5, arg6 );
    /*
     **/
}

void parser_perror(const char* msg, const char* near) {
    fprintf (stderr, "line %d at [%s] (near %s)\n", yyget_lineno(), yyget_text(), near);
    perror (msg);
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

void dlist_add_tail (struct dlist* dest, struct dlist* node) {
    struct dlist* next_node = dest->next;
    struct dlist* tail_node = dest;

    while (next_node != NULL) {
        tail_node = next_node;
        next_node = tail_node->next;
    }
    dlist_add_next(tail_node, node);
}

int dlist_remove (struct dlist* node) {
    struct dlist* next_node = node->next;
    struct dlist* prev_node = node->prev;

    int ret = FALSE;
    if (next_node) {
        ret = TRUE; 
        next_node->prev = prev_node;
    }
    if (prev_node) {
        ret = TRUE; 
        prev_node->next = next_node;
    }
    return ret;
}


