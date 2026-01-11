/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_SCOPE_H
#define GUP_SCOPE_H 1

#include <stdint.h>
#include "gup/token.h"
#include "gup/state.h"

/*
 * Push a scope token onto the scope stack
 *
 * @state:      Compiler state
 * @scope_tok:  Scope token
 *
 * Returns zero on success
 */
int scope_push(struct gup_state *state, tt_t scope_tok);

/*
 * Obtain the most previous scope
 *
 * @state: Compiler state
 */
tt_t scope_top(struct gup_state *state);

/*
 * Pop a scope from the scope stack
 *
 * @state: Compiler state
 */
tt_t scope_pop(struct gup_state *state);

#endif  /* !GUP_SCOPE_H */
