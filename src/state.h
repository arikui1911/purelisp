#ifndef _LISP_INTERNAL_STATE_H_
#define _LISP_INTERNAL_STATE_H_
#include <stddef.h>
#include <setjmp.h>
#include <lisp.h>

struct LispState {
    jmp_buf jbuf;
    LispValue last_error;
    LispObject *heap;
    LispObject **arena;
    size_t arena_len;
    size_t arena_capa;
};

#endif /* _LISP_INTERNAL_STATE_H_ */

