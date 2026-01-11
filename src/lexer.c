/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gup/lexer.h"
#include "gup/trace.h"

/*
 * Returns true if the input character 'c' is
 * a whitespace character
 *
 * @state: Compiler state
 * @c: Character to check against
 */
static bool
lexer_is_ws(struct gup_state *state, char c)
{
    if (state == NULL) {
        return false;
    }

    switch (c) {
    case '\n':
        ++state->line_num;
    case '\r':
    case '\f':
    case '\t':
    case ' ':
        return true;
    }

    return false;
}

/*
 * Consume a single byte from the input source file
 *
 * @state: Compiler state
 * @accept_ws: If true, accept whitespace
 *
 * Returns the character grabbed from the input source
 */
static char
lexer_nom(struct gup_state *state, bool accept_ws)
{
    char c;

    if (state == NULL) {
        return '\0';
    }

    while (read(state->in_fd, &c, 1) > 0) {
        if (lexer_is_ws(state, c) && !accept_ws) {
            continue;
        }

        return c;
    }

    return '\0';
}

/*
 * Scan a line of assembly
 *
 * @state: Compiler state
 * @res: Token result written here
 *
 * Returns zero on success
 */
static int
lexer_scan_asm(struct gup_state *state, struct token *res)
{
    char *buf;
    size_t buf_cap;
    size_t buf_size;
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    buf_cap = 8;
    buf_size = 0;
    if ((buf = malloc(buf_cap)) == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    for (;;) {
        if ((c = lexer_nom(state, true)) == '\0') {
            trace_error(state, "unexpected end of file\n");
            trace_warn("missing a semicolon?\n");
            free(buf);
            return -1;
        }

        /* Is this the end of the assembly? */
        if (c == ';') {
            buf[buf_size] = '\0';
            break;
        }

        buf[buf_size++] = c;
        if (buf_size >= buf_cap - 1) {
            buf_cap += 8;
            buf = realloc(buf, buf_cap);
        }

        if (buf == NULL) {
            return -1;
        }
    }

    res->s = ptrbox_strdup(&state->ptrbox, buf);
    res->type = TT_ASM;
    free(buf);
    return 0;
}

int
lexer_scan(struct gup_state *state, struct token *res)
{
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /* Consume a single byte */
    if ((c = lexer_nom(state, false)) == '\0') {
        return -1;
    }

    switch (c) {
    case '@':
        if (lexer_scan_asm(state, res) < 0) {
            return -1;
        }

        return 0;
    case ';':
        res->type = TT_SEMI;
        res->c = c;
        return 0;
    }

    return -1;
}
