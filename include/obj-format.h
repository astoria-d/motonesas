#ifndef __obj_format_h__
#define __obj_format_h__

#include "tools.h"
#include "segment.h"

/*
MOLF object file format
(MOLF is named after author's name and parody of ELF...)

-------------
MOLF Header
-------------
Segment Header 1
-------------
Segment Header 2
-------------
....
-------------
Segment Header n
-------------
Segment data 1
-------------
Segment data 2 
-------------
....
-------------
Segment data n
-------------
*/

/*
 * MOLF string format:
 * */
struct m_string {
    unsigned short len;
    char* str;
};

/*
 * MOLF header format
 * */
struct molfhdr {
    /*
     * magic number '\117', 'M', 'O', 'L', 'F'
     * */
    unsigned char magic[5];

    /*
     * num of segment 
     * */
    unsigned short seg_cnt;

    /*
     * offset to the segment header from the top
     * */
    unsigned short segh_off;
};

#define MOL_CHK(mh) (mh->magic[0] == '\117' && \
        mh->magic[1] == 'M' && \
        mh->magic[2] == 'O' && \
        mh->magic[3] == 'L' && \
        mh->magic[4] == 'F' )


/*
segment header:
------------
-header size
-segment data position
-segment data size
-num of symbols
 *repeat:
 -symbol name (null terminated)
 -symbol address
-num of unresolved symbols
 *repeat:
 -symbol name (null terminated)
 -referer address
*/

/*
 * Segment header format
 * */
struct seghdr {
    unsigned short segh_size;
    unsigned short seg_start_pos;
    unsigned short seg_data_size;
    char *seg_name;
    short symbol_cnt;
    struct symmap *symbols;
    short unresolve_cnt;
    struct symmap *unresolved_symbols;
};

char* load_string(FILE* fp);
void store_string(FILE* fp, const char* str);

struct molfhdr * load_mh(FILE* fp);
struct seghdr* load_segh(FILE* fp);

#endif /*__obj_format_h__*/

