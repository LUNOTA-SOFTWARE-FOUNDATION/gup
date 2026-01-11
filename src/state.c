/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "gup/state.h"
#include "gup/ptrbox.h"

int
gup_state_init(const char *path, struct gup_state *state)
{
    if (path == NULL || state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    memset(state, 0, sizeof(*state));
    state->in_fd = open(path, O_RDONLY);
    if (state->in_fd < 0) {
        return -1;
    }

    if (ptrbox_init(&state->ptrbox) < 0) {
        close(state->in_fd);
        return -1;
    }

    if (symbol_table_init(&state->symtab) < 0) {
        close(state->in_fd);
        return -1;
    }

    state->out_fp = fopen(DEFAULT_ASMOUT, "w");
    if (state->out_fp == NULL) {
        symbol_table_destroy(&state->symtab);
        close(state->in_fd);
        return -1;
    }

    state->line_num = 1;
    return 0;
}

void
gup_state_destroy(struct gup_state *state)
{
    if (state == NULL) {
        return;
    }

    close(state->in_fd);
    fclose(state->out_fp);
    ptrbox_destroy(&state->ptrbox);
    symbol_table_destroy(&state->symtab);
}
