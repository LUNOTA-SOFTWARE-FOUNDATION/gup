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
#include "gup/ast.h"
#include "gup/codegen.h"
#include "gup/types.h"

#define tokstr(tt)          \
    toktab[(tt)]

#define tokstr1(tok)        \
    tokstr((tok)->type)

#define utok(state, tt)          \
    trace_error(                 \
        state,                   \
        "unexpected token %s\n", \
        tokstr(tt)               \
    );

#define utok1(state, exp, got)           \
    trace_error(                         \
        state,                           \
        "expected %s, got %s instead\n", \
        exp,                             \
        got                              \
    );

#define ueof(state)                 \
    trace_error(                    \
        state,                      \
        "unexpected end of file\n"  \
    );

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
    [TT_LPAREN] = "LPAREN",
    [TT_RPAREN] = "RPAREN",
    [TT_LT]     = "LESS-THAN",
    [TT_GT]     = "GREATER-THAN",
    [TT_U8]     = "U8",
    [TT_U16]    = "U16",
    [TT_U32]    = "U32",
    [TT_U64]    = "U64",
    [TT_PUB]    = "PUB",
    [TT_PROC]   = "PROC",
    [TT_NUMBER] = "NUMBER",
    [TT_IDENT]  = "IDENTIFIER"
};

/*
 * Get a data type from a lexical token type
 *
 * @tt: Token type to test
 *
 * Returns GUP_TYPE_BAD on failure, otherwise the
 * specific data type.
 */
static gup_type_t
parse_get_type(tt_t tt)
{
    switch (tt) {
    case TT_U8:  return GUP_TYPE_U8;
    case TT_U16: return GUP_TYPE_U16;
    case TT_U32: return GUP_TYPE_U32;
    case TT_U64: return GUP_TYPE_U64;
    default:     return GUP_TYPE_BAD;
    }

    return GUP_TYPE_BAD;
}

/*
 * Parse a data type
 *
 * @state: Compiler state
 * @tok:   Last token
 * @res:   Type result written here
 *
 * Returns zero on success
 */
static int
parse_type(struct gup_state *state, struct token *tok, struct datum_type *res)
{
    gup_type_t type;

    type = parse_get_type(tok->type);
    if (type == GUP_TYPE_BAD) {
        utok1(state, "TYPE", tokstr1(tok));
        return -1;
    }

    res->type = type;
    return 0;
}

/*
 * Asserts that the next token is of an expected value
 *
 * @state: Compiler state
 * @tok    Last token
 * @what:  Expected token
 *
 * Returns zero on success
 */
static int
parse_expect(struct gup_state *state, struct token *tok, tt_t what)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    /* This must match */
    if (tok->type != what) {
        utok1(state, tokstr(what), tokstr1(tok));
        return -1;
    }

    return 0;
}

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
    struct ast_node *node;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (ast_alloc_node(state, AST_ASM, &node) < 0) {
        trace_error(state, "failed to allocate AST_ASM\n");
        return -1;
    }

    node->s = tok->s;
    return cg_compile_node(state, node);
}

/*
 * Parse a procedure
 *
 * @state: Compiler state
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_proc(struct gup_state *state, struct token *tok)
{
    struct datum_type type;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (parse_expect(state, tok, TT_IDENT) < 0) {
        return -1;
    }

    if (parse_expect(state, tok, TT_MINUS) < 0) {
        return -1;
    }

    if (parse_expect(state, tok, TT_GT) < 0) {
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if (parse_type(state, tok, &type) < 0) {
        return -1;
    }

    if (parse_expect(state, tok, TT_SEMI) < 0) {
        return -1;
    }
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
    case TT_PROC:
        if (parse_proc(state, tok) < 0) {
            return -1;
        }

        break;
    default:
        utok(state, tok->type);
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
