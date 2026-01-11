/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_AST_H
#define GUP_AST_H 1

#include <stdint.h>
#include <stddef.h>
#include "gup/state.h"
#include "gup/symbol.h"

/*
 * Represents valid AST node types
 *
 * @AST_NONE: No type specified
 * @AST_ASM:  Inline-assembly
 * @AST_PROC: Procedure
 * @AST_LOOP: Loop block
 * @AST_GLOBVAR: Global variable
 */
typedef enum {
    AST_NONE,
    AST_ASM,
    AST_PROC,
    AST_LOOP,
    AST_GLOBVAR
} ast_op_t;

/*
 * Represents a single node within an abstract syntax
 * tree.
 *
 * @type: AST operation type
 * @left: Left node
 * @right: Right node
 * @symbol: Symbol associated with node
 * @epilogue: If set, indicates end of block
 */
struct ast_node {
    ast_op_t type;
    struct ast_node *left;
    struct ast_node *right;
    struct symbol *symbol;
    uint8_t epilogue : 1;
    union {
        char *s;
    };
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
