#include <lisp.h>
#include "state.h"
#include "misc.h"
#include "object.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>

LispState *lisp_open(void){
    LispState *s = malloc(sizeof(LispState));
    if (!s) return NULL;
    s->heap = NULL;
    s->arena = NULL;
    s->arena_len = 0;
    s->arena_capa = 0;
    return s;
}

void lisp_close(LispState *lisp){
    if (!lisp) return;
    free(lisp->arena);
    free(lisp);
}

int lisp_guard(LispState *lisp, LispValue *r, LispValue (*fn)(LispState *)){
    if (setjmp(lisp->jbuf) == 0) {
        *r = fn(lisp);
        return 0;
    }
    *r = lisp->last_error;
    return -1;
}

void lisp_error(LispState *lisp, const char *fmt, ...){
    va_list args;

    lisp->last_error.type = LISP_CONDITION;
    lisp->last_error.as.obj = object_alloc(lisp, LISP_CONDITION);

    va_start(args, fmt);
    lisp->last_error.as.obj->as.cond.message = lisp_string_vformat(lisp, fmt, args);
    va_end(args);

    longjmp(lisp->jbuf, 1);
}

void *lrealloc(LispState *lisp, void *p, size_t n){
    void *r = realloc(p, n);
    if (!r) {
        lisp_gc(lisp);
        r = realloc(p, n);
    }
    if (!r) lisp_error(lisp, "out of memory");
    return r;
}

