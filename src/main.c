#include <stdio.h>
#include <lisp.h>

int main(void){
    LispState *lisp;
    LispValue v;
    
    lisp = lisp_open();
    if (!lisp) return -1;

    v = lisp_string_format(lisp, "<<<%s>>>", "HOGEEEEE");

    lisp_close(lisp);
    return 0;
}

void on_bol(void) {
    fprintf(stderr, "Lisp >");
}

int main0(void) {
    int c;
    int on_eol = 1;

    //whlie ((c = fgetc(stdin)) != EOF) {
    for (;;) {
        if (on_eol) {
            on_eol = 0;
            on_bol();
        }

        c = fgetc(stdin);
        if (c == EOF) break;
        if (c == '\n') on_eol = 1;
    }

    return 0;
}

