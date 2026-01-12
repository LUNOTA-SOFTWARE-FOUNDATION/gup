/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
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

static int
cg_emit_proc(struct gup_state *state, struct ast_node *node)
{
    struct symbol *symbol;

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_PROC) {
        errno = -EINVAL;
        return -1;
    }

    if (node->epilogue) {
        mu_cg_ret(state);
        return 0;
    }

    if ((symbol = node->symbol) == NULL) {
        errno = -EIO;
        return -1;
    }

    if (node->s != NULL) {
        mu_cg_label(state, node->s, symbol->global);
    }

    return 0;
}

/*
 * Emit a loop
 *
 * @state: Compiler state
 * @node:  Loop node
 *
 * Returns zero on success
 */
static int
cg_emit_loop(struct gup_state *state, struct ast_node *node)
{
    char label_buf[32];

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /*
     * If this is not a loop epilogue, generate the
     * loop start label and that's it.
     */
    if (!node->epilogue) {
        snprintf(
            label_buf,
            sizeof(label_buf),
            "L.%zu",
            state->loop_count++
        );

        mu_cg_label(state, label_buf, false);
        return 0;
    }

    /* Emit a jump to the start label */
    snprintf(label_buf, sizeof(label_buf), "L.%zu", state->loop_count - 1);
    mu_cg_jmp(state, label_buf);

    /* Emit the end label */
    snprintf(label_buf, sizeof(label_buf), "L.%zu.1", state->loop_count - 1);
    mu_cg_label(state, label_buf, false);
    return 0;
}

/*
 * Emit a global variable
 *
 * @state: Compiler state
 * @node:  Node of global variable
 */
static int
cg_emit_globvar(struct gup_state *state, struct ast_node *node)
{
    struct datum_type *dtype;
    struct symbol *symbol;
    msize_t msize;

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((symbol = node->symbol) == NULL) {
        errno = -EIO;
        return -1;
    }

    dtype = &symbol->data_type;

    /*
     * If we are dealing with a regular type, keep the size
     * as is. However if it is a pointer, then it shall be
     * promoted to the largest type.
     */
    if (dtype->ptr_depth > 0) {
        msize = MSIZE_QWORD;
    } else {
        msize = type_to_msize(dtype->type);
    }

    return mu_cg_var(state, SECTION_DATA, symbol->name, msize, 0);
}

/*
 * Emit a break statement
 *
 * @state: Compiler state
 * @node:  Node of break statement
 */
static int
cg_emit_break(struct gup_state *state, struct ast_node *node)
{
    char label_buf[32];

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_BREAK) {
        errno = -EINVAL;
        return -1;
    }

    snprintf(
        label_buf,
        sizeof(label_buf),
        "L.%zu.1",
        state->loop_count - 1
    );

    return mu_cg_jmp(state, label_buf);
}

/*
 * Emit a continue statement
 *
 * @state: Compiler state
 * @node:  Node of continue statement
 */
static int
cg_emit_continue(struct gup_state *state, struct ast_node *node)
{
    char label_buf[32];

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_CONTINUE) {
        errno = -EINVAL;
        return -1;
    }

    snprintf(
        label_buf,
        sizeof(label_buf),
        "L.%zu",
        state->loop_count - 1
    );

    return mu_cg_jmp(state, label_buf);
}

/*
 * Emit a procedure call
 *
 * @state: Compiler state
 * @node:  Node of procedure call
 */
static int
cg_emit_call(struct gup_state *state, struct ast_node *node)
{
    struct symbol *symbol;

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_CALL) {
        errno = -EINVAL;
        return -1;
    }

    if ((symbol = node->symbol) == NULL) {
        errno = -EIO;
        return -1;
    }

    if (symbol->type != SYMBOL_FUNC) {
        trace_error(state, "'%s' is not a function\n", symbol->name);
        return -1;
    }

    return mu_cg_call(state, symbol->name);
}

/*
 * Emit a return
 *
 * @state: Compiler state
 * @node:  Node of return
 */
static int
cg_emit_ret(struct gup_state *state, struct ast_node *node)
{
    struct datum_type *dtype;
    struct symbol *symbol;
    msize_t msize;

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_RET) {
        errno = -EINVAL;
        return -1;
    }

    if ((symbol = state->this_func) == NULL) {
        errno = -EIO;
        return -1;
    }

    /* Promote type if needed */
    dtype = &symbol->data_type;
    if (dtype->ptr_depth > 0) {
        msize = MSIZE_QWORD;
    } else {
        msize = type_to_msize(dtype->type);
    }

    return mu_cg_retimm(state, msize, node->v);
}

/*
 * Emit a struct
 *
 * @state: Compiler state
 * @node:  Node of struct
 */
static int
cg_emit_struct(struct gup_state *state, struct ast_node *node)
{
    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_STRUCT) {
        errno = -EINVAL;
        return -1;
    }

    return mu_cg_struct(state, node);
}

/*
 * Emit a struct access
 *
 * @state: Compiler state
 * @node:  Node of struct access
 */
static int
cg_emit_access(struct gup_state *state, struct ast_node *node)
{
    struct ast_node *cur;

    if (state == NULL || node == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (node->type != AST_ACCESS) {
        errno = -EINVAL;
        return -1;
    }

    cur = node;
    printf("detected access of [");
    while (cur != NULL) {
        printf("%s", cur->s);
        if ((cur = cur->right) != NULL)
            printf(".");
    }

    printf("]\n");
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
    case AST_PROC:
        if (cg_emit_proc(state, node) < 0) {
            return -1;
        }

        break;
    case AST_LOOP:
        if (cg_emit_loop(state, node) < 0) {
            return -1;
        }

        break;
    case AST_GLOBVAR:
        if (cg_emit_globvar(state, node) < 0) {
            return -1;
        }

        break;
    case AST_BREAK:
        if (cg_emit_break(state, node) < 0) {
            return -1;
        }

        break;
    case AST_CONTINUE:
        if (cg_emit_continue(state, node) < 0) {
            return -1;
        }

        break;
    case AST_CALL:
        if (cg_emit_call(state, node) < 0) {
            return -1;
        }

        break;
    case AST_RET:
        if (cg_emit_ret(state, node) < 0) {
            return -1;
        }

        break;
    case AST_STRUCT:
        if (cg_emit_struct(state, node) < 0) {
            return -1;
        }

        break;
    case AST_ACCESS:
        if (cg_emit_access(state, node) < 0) {
            return -1;
        }

        break;
    default:
        trace_error(state, "bad AST node [type=%d]\n", node->type);
        return -1;
    }

    return 0;
}
