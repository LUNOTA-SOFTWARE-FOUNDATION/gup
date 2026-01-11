/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>
#include "gup/lexer.h"

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
        res->type = TT_AT;
        res->c = c;
        return 0;
    case ';':
        res->type = TT_SEMI;
        res->c = c;
        return 0;
    }

    return -1;
}
