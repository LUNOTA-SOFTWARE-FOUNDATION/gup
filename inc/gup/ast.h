/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_AST_H
#define GUP_AST_H 1

#include <stdint.h>
#include <stddef.h>
#include "gup/state.h"

/*
 * Represents valid AST node types
 *
 * @AST_NONE: No type specified
 */
typedef enum {
    AST_NONE
} ast_op_t;

/*
 * Represents a single node within an abstract syntax
 * tree.
 *
 * @type: AST operation type
 * @left: Left node
 * @right: Right node
 */
struct ast_node {
    ast_op_t type;
    struct ast_node *left;
    struct ast_node *right;
};

/*
 * Allocate an abstract syntax tree node
 *
 * @state: Compiler state
 * @type: Node type
 * @res: Result is written here
 */
int ast_alloc_node(
    struct gup_state *state, ast_op_t type,
    struct ast_node **res
);

#endif  /* !GUP_AST_H */
