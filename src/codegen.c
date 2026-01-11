/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "gup/trace.h"
#include "gup/codegen.h"
#include "gup/mu.h"

/*
 * Emit inline-assembly from an AST node
 *
 * @state: Compiler state
 * @node:  Node to generate assembly from
 *
 * Returns zero on success
 */
static int
cg_emit_asm(struct gup_state *state, struct ast_node *node)
{
    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->s != NULL) {
        mu_cg_inject(state, node->s);
    }
    return 0;
}

int
cg_compile_node(struct gup_state *state, struct ast_node *node)
{
    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    switch (node->type) {
    case AST_ASM:
        if (cg_emit_asm(state, node) < 0) {
            return -1;
        }

        break;
    default:
        trace_error(state, "bad AST node [type=%d]\n", node->type);
        return -1;
    }

    return 0;
}
