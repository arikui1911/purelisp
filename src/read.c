#include <lisp.h>
#include "./misc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

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
    const char *filename;
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

static LispReader *new_reader(LispState *lisp, SourceType src_type, const char *src_filename){
    LispReader *r = lmalloc(lisp, sizeof(LispReader));
    r->lisp = lisp;
    r->src_type = src_type;
    r->filename = src_filename;
    if (!r->filename) r->filename = "";
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

 LispReader *lisp_read_io(LispState *lisp, FILE *src, const char *filename, const char *prompt){
    LispReader *r = new_reader(lisp, SRC_FILE, filename);
    r->src.file = src;
    r->prompt = prompt;
    return r;
}

LispReader *lisp_read_string(LispState *lisp, const char *src, const char *filename){
    LispReader *r = new_reader(lisp, SRC_STRING, filename);
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
    STATE_STRING_ESC,
    STATE_ATOM,
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
                reset_buffer(r);
                state = STATE_STRING;
                t.tag = TOKEN_STRING;
                break;
            case '(':
                t.tag = TOKEN_LP;
                goto LOOP_END;
            case ')':
                t.tag = TOKEN_RP;
                goto LOOP_END;
            case '.':
                t.tag = TOKEN_DOT;
                goto LOOP_END;
            default:
                if (!isspace(c)) {
                    reset_buffer(r);
                    add_to_buffer(r, toupper(c));
                    state = STATE_ATOM;
                    t.tag = TOKEN_ATOM;
                }
            }
            break;
        case STATE_COMMENT:
            if (c == '\n') {
                r->lineno++;
                state = STATE_INITIAL;
            }
            break;
        case STATE_STRING:
            switch (c) {
            case '\\':
                state = STATE_STRING_ESC;
                break;
            case '"':
                add_to_buffer(r, '\0');
                state = STATE_INITIAL;
                goto LOOP_END;
            case '\n':
                r->lineno++;
                add_to_buffer(r, c);
                break;
            default:
                add_to_buffer(r, c);
            }
            break;
        case STATE_STRING_ESC:
            switch (c) {
            case '\n':
                break;
            case 'n':
                add_to_buffer(r, '\n');
                break;
            case 't':
                add_to_buffer(r, '\t');
                break;
            default:
                add_to_buffer(r, c);
            }
            state = STATE_STRING;
            break;
        case STATE_ATOM:
            switch (c) {
            case '\n':
                r->lineno++;
                goto LOOP_END;
            case ';':
            case '"':
            case '(':
            case ')':
                ungetch(r, c);
                goto LOOP_END;
            default:
                if (isspace(c)) goto LOOP_END;
                add_to_buffer(r, toupper(c));
            }
            break;
        default:
            assert(0);
        }
    }
LOOP_END:

    switch (state) {
    case STATE_STRING:
        lisp_error(r->lisp, "%s:%d:%d: unterminated string literal", r->filename, t.lineno, t.column);
        break;
    case STATE_ATOM:
        add_to_buffer(r, '\0');
        break;
    default:
        break;
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

static LispValue parse_atom(LispReader *r, Token t){
    if (strcmp(t.value, "T") == 0) return lisp_t_value();
    if (strcmp(t.value, "nil") == 0) return lisp_nil_value();

}

static LispValue read_list(LispReader *r){
    LispValue car;
    Token t;

    t = get_token(r);
    switch (t.tag) {
    case TOKEN_RP:
        return lisp_nil_value();
    case TOKEN_EOF:
        lisp_error(r->lisp, "%s:%d:%d: unterminated list", r->filename, t.lineno, t.column);
        break;
    default:
        break;
    }
    unget_token(r, t);
    car = lisp_read(r);

    t = get_token(r);
    if (t.tag == TOKEN_DOT) {
        LispValue cdr = lisp_read(r);
        t = get_token(r);
        if (t.tag != TOKEN_RP) {
            lisp_error(r->lisp, "%s:%d:%d: invalid dot list", r->filename, t.lineno, t.column);
        }
        return lisp_cons(r->lisp, car, cdr);
    }
    unget_token(r, t);

    return lisp_cons(r->lisp, car, lisp_read(r));
}

LispValue lisp_read(LispReader *r) {
    Token t;
    t = get_token(r);
    switch (t.tag) {
    case TOKEN_EOF:
        return lisp_eof_value();
    case TOKEN_ATOM:
        return parse_atom(r, t);
    case TOKEN_STRING:
        return lisp_string_new(r->lisp, t.value, strlen(t.value));
    case TOKEN_LP:
        return read_list(r);
    case TOKEN_RP:
    case TOKEN_DOT:
        lisp_error(r->lisp, "%s:%d:%d: unexpected token", r->filename, t.lineno, t.column);
        break;
    default:
        assert(0);
    }
}

