
#include <stdio.h>
#include <stdlib.h>
#include "tools.h"

#define BUF_SIZE 100

struct lmap_entry {
    struct dlist list;
    char* seg_name;
    unsigned short start;
    unsigned short size;
};

static struct lmap_entry * lmap_list;


int lookup_lmap(const char* segname, unsigned short *start, unsigned short *size) {
    struct lmap_entry *lm;
    //dprint("lookup_lmap %s\n", segname);

    lm = lmap_list;
    while (lm != NULL) {
        //dprint(">>: %s\n", lm->seg_name);
        if (!strcmp(segname, lm->seg_name)) {
            *start = lm->start;
            *size = lm->size;
            return TRUE;
        }
        lm = (struct lmap_entry*)lm->list.next;
    }
    return FALSE;
}

int load_lmap(const char* fname) {
    FILE* fp;
    char ch;
    char seg_name [100];
    int start, size;
    int scan_cnt;
    char* buf;

    fp = fopen (fname, "r");
    if (fp == NULL)
        return FALSE;


    while ( fread(&ch, 1, 1, fp) > 0 ){
        struct lmap_entry* lm;
        //read line by line.
        if (ch == '#') {
            while ( fread(&ch, 1, 1, fp) > 0 ){
                if (ch == '\n')
                    break;
            }
            continue;
        }
        else {
            int buf_cnt = 1;
            int i = 0;
            int size = BUF_SIZE;

            buf = malloc (size);
            buf [i++] = ch;
            //dprint("%c\n", ch);
            while ( fread(&ch, 1, 1, fp) > 0 ){
                if (i == BUF_SIZE * buf_cnt) {
                    //dprint("realloc: %d\n", BUF_SIZE * buf_cnt);
                    buf = realloc(buf, BUF_SIZE * ++buf_cnt);
                }
                if (ch == '\n') {
                    buf [i] = '\0';
                    break;
                }
                //dprint("%c\n", ch);
                buf [i++] = ch;
            }
            buf [i] = '\0';
            //dprint("buf: %s\n", buf);
        }

        //skip blank line 
        if (*buf == '\n') {
            free(buf);
            continue;
        }

        scan_cnt = sscanf(buf, "%s %x %x", seg_name, &start, &size);
        if (scan_cnt != 3) {
            free (buf);
            return FALSE;
        }
        //dprint("seg: %s %04x, %04x\n", seg_name, start, size);

        //free (buf);
        lm = malloc(sizeof(struct lmap_entry));
        dlist_init(&lm->list);
        lm->seg_name = strdup(seg_name);
        lm->start = start;
        lm->size = size;

        if (!lmap_list) {
            lmap_list = lm;
        }
        else {
            dlist_add_tail(lmap_list, lm);
        }
    }

    return TRUE;
}

static void clear_lmap(void) {
    struct lmap_entry *lm;
    lm = lmap_list;
    while (lm != NULL) {
        struct lmap_entry *pp;

        pp = lm;
        lm = (struct lmap_entry*)lm->list.next;
        dlist_remove(&pp->list);
        free(pp->seg_name);
        free(pp);
    }

}

int init_lmap(void) {
    lmap_list = NULL;
    return TRUE;
}

void destory_lmap(void) {
    clear_lmap();
}

