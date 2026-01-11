/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_MU_H
#define GUP_MU_H 1

#include <stdbool.h>
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

/*
 * Create an assembly label
 *
 * @state: Compiler state
 * @s: Name of label to create
 * @is_global: If true, label becomes global
 *
 * Returns zero on success
 */
int mu_cg_label(struct gup_state *state, const char *s, bool is_global);

/*
 * Emit a return instruction or architectural equivalent
 *
 * @state: Compiler state
 *
 * Returns zero on success
 */
int mu_cg_ret(struct gup_state *state);

#endif  /* !GUP_MU_H */
