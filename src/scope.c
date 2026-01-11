/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "gup/trace.h"
#include "gup/scope.h"

int
scope_push(struct gup_state *state, tt_t scope_tok)
{
    if (state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (state->scope_depth >= MAX_SCOPE_DEPTH) {
        trace_error(state, "maximum scope depth reached\n");
        return -1;
    }

    state->scope_stack[state->scope_depth++] = scope_tok;
    return 0;
}

tt_t
scope_top(struct gup_state *state)
{
    if (state == NULL) {
        return TT_NONE;
    }

    if (state->scope_depth == 0) {
        return state->scope_stack[0];
    }

    return state->scope_stack[state->scope_depth - 1];
}

tt_t
scope_pop(struct gup_state *state)
{
    tt_t scope;

    if (state->scope_depth == 0) {
        return state->scope_stack[0];
    }

    scope = state->scope_stack[--state->scope_depth];
    state->scope_stack[state->scope_depth] = TT_NONE;
    return scope;
}
