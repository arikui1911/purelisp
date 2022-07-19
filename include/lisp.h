#ifndef _LISP_H_
#define _LISP_H_
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

typedef struct LispValue LispValue;

typedef enum {
    LISP_NIL,
    LISP_T,
    LISP_CELL,
    LISP_SYMBOL,
    LISP_OPERATOR,
    LISP_CONDITION,
    LISP_INT,
    LISP_FLOAT,
    LISP_STRING,
} LispValueType;

typedef struct LispObject LispObject;

struct LispValue {
    LispValueType type;
    union {
        int int_value;
        double flo_value;
        LispObject *obj;
    } as;
};

LispValue lisp_t_value(void);
LispValue lisp_nil_value(void);
LispValue lisp_int_value(int);
LispValue lisp_float_value(double);

typedef struct LispState LispState;

LispState *lisp_open(void);
void lisp_close(LispState *);

int lisp_guard(
    LispState *, LispValue *,
    LispValue (*)(LispState *)
);
void lisp_error(LispState *, const char *, ...);

void lisp_gc(LispState *);

LispValue lisp_string_new(LispState *, const char *, size_t);
LispValue lisp_string_format(LispState *, const char *, ...);
LispValue lisp_string_vformat(LispState *, const char *, va_list);
LispValue lisp_intern(LispState *, LispValue);
LispValue lisp_cons(LispState *, LispValue, LispValue);


typedef struct LispReader LispReader;

LispReader *lisp_read_io(LispState *, FILE *, const char *);
LispReader *lisp_read_string(LispState *, const char *);
void lisp_reader_close(LispReader *);


#endif /* _LISP_H_ */

