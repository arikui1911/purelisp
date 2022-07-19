#ifndef _LISP_INTERNAL_OBJECT_H_
#define _LISP_INTERNAL_OBJECT_H_
#include <lisp.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    char *data;
    size_t len;
} String;

typedef struct {
    LispValue car;
    LispValue cdr;
} Cell;

typedef struct {
    LispValue message;
} Condition;

struct LispObject {
    LispValueType type;
    int mark;
    LispObject *next;
    union {
        String str;
        Cell cell;
        Condition cond;
    } as;
};

void value_mark(LispState *, LispValue);

LispObject *object_alloc(LispState *, LispValueType);
void object_mark(LispState *, LispObject *);
void object_free(LispState *, LispObject *);

#define string_destroy(l, s)        (free((s)->data))
#define cell_destroy(l, c)          do{}while(0)
#define condition_destriy(l, c)     do{}while(0)

#endif /* _LISP_INTERNAL_OBJECT_H_ */

