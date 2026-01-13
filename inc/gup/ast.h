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
 * @AST_BREAK:   Break statement
 * @AST_CONTINUE: Continue statement
 * @AST_CALL:    Procedure call
 * @AST_RET:     Return statement
 * @AST_STRUCT:  Structure
 * @AST_FIELD:   Field
 * @AST_INSTANCE: Instance
 * @AST_ACCESS: Structure access
 * @AST_ASSIGN: Assignment
 * @AST_NUMBER: A number
 * @AST_EQUALITY: Eqaulity operator
 * @AST_IF: If statement
 */
typedef enum {
    AST_NONE,
    AST_ASM,
    AST_PROC,
    AST_LOOP,
    AST_GLOBVAR,
    AST_BREAK,
    AST_CONTINUE,
    AST_CALL,
    AST_RET,
    AST_STRUCT,
    AST_FIELD,
    AST_ACCESS,
    AST_ASSIGN,
    AST_NUMBER,
    AST_EQUALITY,
    AST_IF,
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
 * @field_type: Used in structure fields
 */
struct ast_node {
    ast_op_t type;
    struct ast_node *left;
    struct ast_node *right;
    struct symbol *symbol;
    uint8_t epilogue : 1;
    gup_type_t field_type;
    union {
        char *s;
        ssize_t v;
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
