#include "tools.h"
#include "6502directive.h"
#include <stdio.h>

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

typedef int (check_func_t) (const char* directive, int param_type, const char* str, int num) ;

struct directive_check_func{
    char* directive;
    check_func_t * func;
};

#define DIR_CHK_ENTRY(X)    {#X, X##_FUNC}
#define DIR_CHK_FUNC_DECL(X)    check_func_t X##_FUNC
#define DIR_CHK_FUNC(X)    int X##_FUNC (const char* directive, \
        int param_type, const char* str, int num)

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

FILE * get_current_file(void);
void move_current_pc(short offset);
void add_unresolved_ref(const char* symbol);

int directive_check (const char* directive, int param_type, const char* str, int num) {
    const char* pdir;
    int i;
    int found = FALSE;

    for (i = 0; i < DIRECTIVE_CNT; i++) {
        pdir = dir_check_tbl[i].directive; 
        if (!strcmp(pdir, directive)) {
            found = TRUE;
            return dir_check_tbl[i].func (directive, param_type, str, num);
        }
    }
    return found;
}

DIR_CHK_FUNC(autoimport) {
    /*
     * syntax:
     * .autoimport on
     * */
    if (param_type != DIR_PARAM_IDENT)
        return FALSE;
    if (strcmp(str, "on"))
        return FALSE;
    return TRUE;
}

DIR_CHK_FUNC(endproc) {
    /*
     * syntax:
     * .endproc
     * */
    if (param_type != DIR_PARAM_NON)
        return FALSE;

    return TRUE;
}

DIR_CHK_FUNC(incbin) {
    /*
     * syntax:
     * .incbin "file name"
     * */
    const char* fname;
    FILE *incf;
    FILE *outf;
    int len = 0;
    char ch;
    if (param_type != DIR_PARAM_LITERAL)
        return FALSE;

    fname = str;
    dprint("include %s\n", fname);
    incf = fopen(fname, "r");
    if (incf == NULL)
        return FALSE;

    outf = get_current_file();
    while (fread(&ch, 1, 1, incf) > 0) {
        fwrite(&ch, 1, 1, outf);
        len++;
    }
    close(incf);
    move_current_pc(len);

    return TRUE;
}

DIR_CHK_FUNC(proc) {
    /*
     * syntax:
     * .proc  procname
     * */
    if (param_type != DIR_PARAM_IDENT)
        return FALSE;

    //add procname symbol
    return add_symbol(str);
}

DIR_CHK_FUNC(segment) {
    /*
     * syntax:
     * .segment  "segment name"
     * */
    if (param_type != DIR_PARAM_LITERAL)
        return FALSE;

    set_segment(str);
    return TRUE;
}

DIR_CHK_FUNC(setcpu) {
    /*
     * syntax:
     * .setcpu  "6502"
     * */
    if (param_type != DIR_PARAM_LITERAL)
        return FALSE;
    if (strcmp(str, "6502"))
        return FALSE;
    return TRUE;
}


DIR_CHK_FUNC(byte) {
    if (param_type == DIR_PARAM_LITERAL) {
        /*
         * syntax:
         * .byte  "ascii string"
         * */
        write_str(str);
        return TRUE;
    }
    else if (param_type == DIR_PARAM_NUM) {
        return TRUE;
    }
    return FALSE;
}

DIR_CHK_FUNC(word) {
    if (param_type == DIR_PARAM_NUM) {
        /*
         * syntax:
         * .word    number
         * */
        //write_word_data(num);
        //write_word is called by the caller parser.
        //(because word data may be chained with comma.)
        return TRUE;
    }
    if (param_type == DIR_PARAM_IDENT) {
        /*
         * syntax:
         * .word    symbol
         * */
        unsigned short addr = 0;
        int ret;
        ret = addr_lookup(str, &addr); 
        if (!ret) {
            add_unresolved_ref(str);
        }
        deb_print_addr_feed();
        write_word_data(addr);
        deb_print_nl(); 
        return TRUE;
    }
    return TRUE;
}

