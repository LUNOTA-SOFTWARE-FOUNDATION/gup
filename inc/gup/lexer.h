/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_LEXER_H
#define GUP_LEXER_H 1

#include "gup/state.h"
#include "gup/token.h"

/*
 * Scan for a single token within the source input
 *
 * @state: Compiler state
 * @res: Token is written here
 *
 * Returns zero on success
 */
int lexer_scan(struct gup_state *state, struct token *res);

#endif  /* !GUP_LEXER_H */
