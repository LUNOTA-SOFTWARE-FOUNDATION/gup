/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_MU_H
#define GUP_MU_H 1

#include "gup/state.h"

/*
 * Inject assembly into the program
 *
 * @state: Compiler state
 * @str: Assembly to inject
 *
 * Returns zero on success
 */
int mu_cg_inject(struct gup_state *state, const char *str);

#endif  /* !GUP_MU_H */
