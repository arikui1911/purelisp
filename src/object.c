#include <lisp.h>
#include "state.h"
#include "object.h"
#include "misc.h"
#include <stdlib.h>
#include <assert.h>

#define ARENA_INCREMENTAL (64)

LispObject *object_alloc(LispState *lisp, LispValueType type) {
    LispObject *o = lmalloc(lisp, sizeof(LispObject));

    if (lisp->arena_len == lisp->arena_capa) {
        lisp->arena = lrealloc(
            lisp,
            lisp->arena,
            lisp->arena_capa + ARENA_INCREMENTAL
        );
    }

    o->mark = 1;
    o->type = type;
    lisp->arena[lisp->arena_len++] = o;
    o->next = lisp->heap;
    lisp->heap = o;
    return o;
}

void value_mark(LispState *lisp, LispValue v) {
    switch (v.type) {
    case LISP_CELL:
    case LISP_STRING:
    case LISP_CONDITION:
        object_mark(lisp, v.as.obj);
        break;
    default:
        break;
    }
}

void object_mark(LispState *lisp, LispObject *o) {
    o->mark = 1;
    switch (o->type) {
    case LISP_CELL:
        value_mark(lisp, o->as.cell.car);
        value_mark(lisp, o->as.cell.cdr);
        break;
    case LISP_STRING:
        break;
    case LISP_CONDITION:
        value_mark(lisp, o->as.cond.message);
        break;
    default:
        assert(0);
    }
}

void object_free(LispState *lisp, LispObject *o) {
    switch (o->type) {
    case LISP_CELL:
        cell_destroy(lisp, &o->as.cell);
        break;
    case LISP_STRING:
        string_destroy(lisp, &o->as.str);
        break;
    case LISP_CONDITION:
        condition_destriy(lisp, &o->as.cond);
        break;
    default:
        assert(0);
    }
    free(o);
}

