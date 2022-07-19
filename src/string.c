#include <lisp.h>
#include "object.h"
#include "misc.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

LispValue lisp_string_new(LispState *lisp, const char *str, size_t len) {
    LispValue v = {LISP_STRING};
    v.as.obj = object_alloc(lisp, LISP_STRING);
    v.as.obj->as.str.data = NULL;
    if (len > 0) {
        v.as.obj->as.str.data = lmalloc(lisp, sizeof(char) * len);
        memcpy(v.as.obj->as.str.data, str, len);
    }
    return v;
}

LispValue lisp_string_format(LispState *lisp, const char *fmt, ...){
    LispValue r;
    va_list args;
    va_start(args, fmt);
    r = lisp_string_vformat(lisp, fmt, args);
    va_end(args);
    return r;
}

LispValue lisp_string_vformat(LispState *lisp, const char *fmt, va_list args){
    LispValue v;
    int n;
    va_list copied;

    v = lisp_string_new(lisp, NULL, 0);

    va_copy(copied, args);
    n = vsnprintf(NULL, 0, fmt, copied);
    va_end(copied);

    v.as.obj->as.str.len = n + 1;
    v.as.obj->as.str.data = lmalloc(lisp, sizeof(char) * (n + 1));
    vsnprintf(v.as.obj->as.str.data, n, fmt, args);
    return v;
}

