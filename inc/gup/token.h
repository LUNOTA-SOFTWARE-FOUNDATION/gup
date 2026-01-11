/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_TOKEN_H
#define GUP_TOKEN_H

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
    };
};

#endif  /* !GUP_TOKEN_H */
