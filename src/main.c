#include <stdio.h>
#include <lisp.h>

static LispValue lisp_main(LispState *lisp){
    LispReader *r;
    LispValue v;

    r = lisp_read_io(lisp, stdin, "-", "Lisp >");
    for (;;) {
        v = lisp_read(r);
        if (v.type == LISP_EOF) break;
    }
    lisp_reader_close(r);
    return v;
}

int main(void){
    LispState *lisp;
    LispValue v;
    int stat;
    
    lisp = lisp_open();
    if (!lisp) return -1;

    stat = lisp_guard(lisp, &v, lisp_main);
    if (stat != 0) {
        // error occured
    }

    lisp_close(lisp);
    return stat;
}

