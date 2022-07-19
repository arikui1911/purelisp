#include <lisp.h>
#include "./state.h"
#include "./object.h"

static void clear_marks(LispState *lisp){
    LispObject *pos;
    for (pos = lisp->heap; pos != NULL; pos = pos->next) {
        pos->mark = 0;
    }
}

static void mark_roots(LispState *lisp){
    int i;

    // Last error
    value_mark(lisp, lisp->last_error);

    // Arena
    for (i = 0; i < lisp->arena_len; i++) {
        object_mark(lisp, lisp->arena[i]);
    }

    // Stack
}

static void sweep(LispState *lisp){
    LispObject *pos, *next;
    for (pos = lisp->heap; pos != NULL; pos = next) {
        next = pos->next;
        if (!pos->mark) object_free(lisp, pos);
    }
}

void lisp_gc(LispState *lisp){
    clear_marks(lisp);
    mark_roots(lisp);
    sweep(lisp);
}

