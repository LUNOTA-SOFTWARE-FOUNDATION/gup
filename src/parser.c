/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "gup/parser.h"
#include "gup/token.h"
#include "gup/lexer.h"
#include "gup/trace.h"

/* Most previous input token */
static struct token last_token;

/*
 * A lookup table used to convert token constants
 * to human readable strings.
 */
static const char *toktab[] = {
    [TT_NONE]   = "NONE",
    [TT_ASM]    = "ASM",
    [TT_SEMI]   = "SEMICOLON",
    [TT_STAR]   = "STAR",
    [TT_PLUS]   = "PLUS",
    [TT_MINUS]  = "MINUS",
    [TT_SLASH]  = "SLASH",
    [TT_U8]     = "U8",
    [TT_U16]    = "U16",
    [TT_U32]    = "U32",
    [TT_U64]    = "U64",
    [TT_NUMBER] = "NUMBER",
    [TT_IDENT]  = "IDENTIFIER"
};

/*
 * Handle lines of assembly
 *
 * @state: Compiler state
 * @tok: Last token
 *
 * Returns zero on success
 */
static int
parse_asm(struct gup_state *state, struct token *tok)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    printf("got asm: %s\n", tok->s);
    return 0;
}

/*
 * Begin parsing tokens from the input source
 *
 * @state: Compiler state
 * @tok: Last token
 */
static int
begin_parse(struct gup_state *state, struct token *tok)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    switch (tok->type) {
    case TT_ASM:
        if (parse_asm(state, tok) < 0) {
            return -1;
        }

        break;
    default:
        trace_error(state, "got unexpected token %s\n", toktab[tok->type]);
        return -1;
    }

    return 0;
}

int
gup_parse(struct gup_state *state)
{
    if (state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    while (lexer_scan(state, &last_token) == 0) {
        trace_debug("got token %s\n", toktab[last_token.type]);
        if (begin_parse(state, &last_token) < 0) {
            break;
        }
    }

    return 0;
}
