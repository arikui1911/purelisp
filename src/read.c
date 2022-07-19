#include <lisp.h>
#include "./misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_ATOM,
    TOKEN_STRING,
    TOKEN_LP,
    TOKEN_RP,
    TOKEN_DOT,
} TokenTag;

typedef struct {
    TokenTag tag;
    const char *value;
    int lineno;
    int column;
} Token;

typedef enum {
    SRC_FILE,
    SRC_STRING,
} SourceType;

typedef struct {
    const char *data;
    int len;
    int pos;
} StringSource;

struct LispReader {
    LispState *lisp;
    SourceType src_type;
    union {
        FILE *file;
        StringSource string;
    } src;
    int on_eol;
    const char *prompt;
    int buffered_ch;
    int has_buffered_ch;
    Token buffered_token;
    int has_buffered_token;
    char *buf;
    int buf_len;
    int buf_capa;
    int lineno;
    int column;
};

static LispReader *new_reader(LispState *lisp, SourceType src_type){
    LispReader *r = lmalloc(lisp, sizeof(LispReader));
    r->lisp = lisp;
    r->src_type = src_type;
    r->on_eol = 1;
    r->prompt = NULL;
    r->has_buffered_ch = 0;
    r->has_buffered_token = 0;
    r->buf = NULL;
    r->buf_len = 0;
    r->buf_capa = 0;
    r->lineno = 1;
    r->column = 1;
    return r;
}

LispReader *lisp_read_io(LispState *lisp, FILE *src, const char *prompt){
    LispReader *r = new_reader(lisp, SRC_FILE);
    r->src.file = src;
    r->prompt = prompt;
    return r;
}

LispReader *lisp_read_string(LispState *lisp, const char *src){
    LispReader *r = new_reader(lisp, SRC_STRING);
    r->src.string.data = src;
    r->src.string.len = strlen(src);
    r->src.string.pos = 0;
    return r;
}

void lisp_reader_close(LispReader *r){
    if (!r) return;
    free(r->buf);
    free(r);
}

static int read_char(LispReader *r){
    int c;

    if (r->on_eol) {
        r->on_eol = 0;
        if (r->prompt) fprintf(stderr, "%s", r->prompt);
    }

    switch (r->src_type) {
    case SRC_FILE:
        c = fgetc(r->src.file);
        break;
    case SRC_STRING:
        if (r->src.string.pos == r->src.string.len) {
            c = EOF;
        } else {
            c = r->src.string.data[r->src.string.pos++];
        }
        break;
    default:
        assert(0);
    }
    if (c == '\n') r->on_eol = 1;
    return c;
}

static int getch(LispReader *r){
    if (r->has_buffered_ch) {
        r->has_buffered_ch = 0;
        return r->buffered_ch;
    }
    return read_char(r);
}

static void ungetch(LispReader *r, int c){
    r->buffered_ch = c;
    r->has_buffered_ch = 1;
}

#define reset_buffer(r)     ((r)->buf_len = 0)

#define BUF_INC     (32)

static void add_to_buffer(LispReader *r, int c){
    if (r->buf_len == r->buf_capa) {
        r->buf = lrealloc(r->lisp, r->buf, r->buf_capa + BUF_INC);
        r->buf_capa += BUF_INC;
    }
    r->buf[r->buf_len++] = c;
}

typedef enum {
    STATE_INITIAL,
    STATE_COMMENT,
    STATE_STRING,
} TokenState;

static Token next_token(LispReader *r){
    Token t = {TOKEN_EOF, NULL};
    int c;
    TokenState state = STATE_INITIAL;

    while ((c = getch(r)) != EOF) {
        t.lineno = r->lineno;
        t.column = r->column;
        r->lineno++;
        switch (state) {
        case STATE_INITIAL:
            switch (c) {
            case ';':
                state = STATE_COMMENT;
                break;
            case '"':
                state = STATE_STRING;
                break;
            }
            break;
        case STATE_COMMENT:
            if (c == '\n') {
                r->lineno++;
                state = STATE_INITIAL;
            }
            break;
        case STATE_STRING:
            break;
        default:
            assert(0);
        }
    }

    return t;
}

static Token get_token(LispReader *r){
    if (r->has_buffered_token) {
        r->has_buffered_token = 0;
        return r->buffered_token;
    }
    return next_token(r);
}

static void unget_token(LispReader *r, Token t){
    r->buffered_token = t;
    r->has_buffered_token = 1;
}

