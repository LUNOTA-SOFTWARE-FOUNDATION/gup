/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_PARSER_H
#define GUP_PARSER_H 1

#include "gup/state.h"

/*
 * Begin parsing the source input file
 *
 * @state: Compiler state
 *
 * Returns zero on success
 */
int gup_parse(struct gup_state *state);

#endif  /* !GUP_PARSER_H */
