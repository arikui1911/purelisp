#ifndef _LISP_INTERNAL_UTIL_H_
#define _LISP_INTERNAL_UTIL_H_
#include <lisp.h>
#include <stddef.h>

#define lmalloc(lisp, n)  (lrealloc((lisp), NULL, (n)))
void *lrealloc(LispState *, void *, size_t);

#endif /* _LISP_INTERNAL_UTIL_H_ */

