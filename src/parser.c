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

/* Most previous input token */
static struct token last_token;

/*
 * A lookup table used to convert token constants
 * to human readable strings.
 */
static const char *toktab[] = {
    [TT_NONE]   =  "NONE",
    [TT_ASM]     = "ASM",
    [TT_SEMI]   =  "SEMICOLON"
};

int
gup_parse(struct gup_state *state)
{
    if (state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    while (lexer_scan(state, &last_token) == 0) {
        printf("got token %s\n", toktab[last_token.type]);
    }

    return 0;
}
