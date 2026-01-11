/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_STATE_H
#define GUP_STATE_H

#include "gup/ptrbox.h"

/*
 * Represents the compiler state
 *
 * @in_fd: Source input file descriptor
 * @line_no: Line number
 * @putback: Putback buffer
 * @ptrbox: Global pointer box
 */
struct gup_state {
    int in_fd;
    size_t line_num;
    char putback;
    struct ptrbox ptrbox;
};

/*
 * Initialize the compiler state
 *
 * @path: Source input file path
 * @state: Compiler state
 *
 * Returns zero on success
 */
int gup_state_init(const char *path, struct gup_state *state);

/*
 * Destroy the compiler state
 *
 * @state: Compiler state the destroy
 */
void gup_state_destroy(struct gup_state *state);

#endif  /* !GUP_STATE_H */
