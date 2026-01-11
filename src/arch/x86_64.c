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
        "%s\n",
        str
    );
    return 0;
}
