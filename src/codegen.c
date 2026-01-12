/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <errno.h>
#include "gup/trace.h"
#include "gup/codegen.h"
#include "gup/mu.h"

static inline msize_t
type_to_msize(gup_type_t type)
{
    switch (type) {
    case GUP_TYPE_U8:  return MSIZE_BYTE;
    case GUP_TYPE_U16: return MSIZE_WORD;
    case GUP_TYPE_U32: return MSIZE_DWORD;
    case GUP_TYPE_U64: return MSIZE_QWORD;
    default:           return MSIZE_BAD;
    }

    return MSIZE_BAD;
}

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
    case AST_CALL:
        if (cg_emit_call(state, node) < 0) {
            return -1;
        }

        break;
    default:
        trace_error(state, "bad AST node [type=%d]\n", node->type);
        return -1;
    }

    return 0;
}
