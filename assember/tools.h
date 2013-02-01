
struct slist {
    struct slist *next;
} ;

struct dlist {
    struct dlist *prev;
    struct dlist *next;
} ;


void slist_add_tail (struct slist* dest, struct slist* node) ;

void dlist_init (struct dlist* node) ;
void dlist_add_next (struct dlist* dest, struct dlist* node) ;
void dlist_remove (struct dlist* node) ;

#define TRUE 1
#define FALSE 0

#define R_OK 0
#define R_ERROR -1
