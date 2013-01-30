
struct slist {
    struct slist *next;
} ;


void list_init (struct slist* top) ;
void slist_add_tail (struct slist* dest, struct slist* node) ;

