/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_TOKEN_H
#define GUP_TOKEN_H

#include <sys/types.h>

/*
 * Represents valid token types
 */
typedef enum {
    TT_NONE,        /* <NONE> */
    TT_ASM,         /* '@' */
    TT_SEMI,        /* ';' */
    TT_STAR,        /* '*' */
    TT_PLUS,        /* '+' */
    TT_MINUS,       /* '-' */
    TT_SLASH,       /* '/' */
    TT_LPAREN,      /* '(' */
    TT_RPAREN,      /* ')' */
    TT_LBRACE,      /* '{' */
    TT_RBRACE,      /* '}' */
    TT_LT,          /* '<' */
    TT_GT,          /* '>' */
    TT_U8,          /* 'u8' */
    TT_U16,         /* 'u16' */
    TT_U32,         /* 'u32' */
    TT_U64,         /* 'u64' */
    TT_VOID,        /* 'void' */
    TT_PUB,         /* 'pub' */
    TT_PROC,        /* 'proc' */
    TT_LOOP,        /* 'loop' */
    TT_BREAK,       /* 'break' */
    TT_RETURN,      /* 'return' */
    TT_STRUCT,      /* 'struct' */
    TT_NUMBER,      /* <NUMBER> */
    TT_IDENT,       /* <IDENTIFIER> */
} tt_t;

/*
 * Represents a single lexical element within the
 * source input.
 *
 * @type: Type of this token
 */
struct token {
    tt_t type;
    union {
        char c;
        char *s;
        ssize_t v;
    };
};

#endif  /* !GUP_TOKEN_H */
