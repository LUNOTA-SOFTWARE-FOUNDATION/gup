/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "gup/mu.h"

int
mu_cg_inject(struct gup_state *state, const char *str)
{
    if (state == NULL || str == NULL) {
        errno = -EINVAL;
        return -1;
    }

    fprintf(
        state->out_fp,
        "\t%s\n",
        str
    );
    return 0;
}

int
mu_cg_label(struct gup_state *state, const char *s, bool is_global)
{
    if (state == NULL || s == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (is_global) {
        fprintf(
            state->out_fp,
            "[global %s]\n",
            s
        );
    }

    fprintf(
        state->out_fp,
        "%s:\n",
        s
    );

    return 0;
}
