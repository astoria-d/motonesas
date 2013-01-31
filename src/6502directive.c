#include "tools.h"
#include "6502directive.h"

/**
 * supported directive:
 * .autoimport
 * .byte
 * .endproc
 * .incbin
 * .proc
 * .segment
 * .setcpu
 * .word
 *
 * # grep ".*\." sample1.asm  | awk '{print $1}'| sort |uniq
 *
 */

typedef int (check_func_t) (const char* directive, int param_type, const char* str) ;

struct directive_check_func{
    char* directive;
    check_func_t * func;
};

#define DIR_CHK_ENTRY(X)    {#X, X##_FUNC}
#define DIR_CHK_FUNC_DECL(X)    check_func_t X##_FUNC
#define DIR_CHK_FUNC(X)    int X##_FUNC (const char* directive, int param_type, const char* str)

DIR_CHK_FUNC_DECL(autoimport);
DIR_CHK_FUNC_DECL(byte); 
DIR_CHK_FUNC_DECL(endproc); 
DIR_CHK_FUNC_DECL(incbin); 
DIR_CHK_FUNC_DECL(proc); 
DIR_CHK_FUNC_DECL(segment); 
DIR_CHK_FUNC_DECL(setcpu); 
DIR_CHK_FUNC_DECL(word); 

static struct directive_check_func dir_check_tbl[] = {
DIR_CHK_ENTRY(autoimport), 
DIR_CHK_ENTRY(byte), 
DIR_CHK_ENTRY(endproc), 
DIR_CHK_ENTRY(incbin), 
DIR_CHK_ENTRY(proc), 
DIR_CHK_ENTRY(segment), 
DIR_CHK_ENTRY(setcpu), 
DIR_CHK_ENTRY(word), 
};

#define DIRECTIVE_CNT   sizeof (dir_check_tbl) / sizeof (struct directive_check_func)

int directive_check (const char* directive, int param_type, const char* str) {
    const char* pdir;
    int i;
    int found = FALSE;

    for (i = 0; i < DIRECTIVE_CNT; i++) {
        pdir = dir_check_tbl[i].directive; 
        if (!strcmp(pdir, directive)) {
            found = TRUE;
            break;
        }
    }
    return found;
}

DIR_CHK_FUNC(autoimport) {
}

DIR_CHK_FUNC(byte) {
}

DIR_CHK_FUNC(endproc) {
}

DIR_CHK_FUNC(incbin) {
}

DIR_CHK_FUNC(proc) {
}

DIR_CHK_FUNC(segment) {
}

DIR_CHK_FUNC(setcpu) {
}

DIR_CHK_FUNC(word) {
}

