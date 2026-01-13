/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_MU_H
#define GUP_MU_H 1

#include <stdbool.h>
#include "gup/state.h"
#include "gup/ast.h"

/*
 * Valid machine sizes
 */
typedef enum {
    MSIZE_BAD,
    MSIZE_BYTE,     /* 8 bits */
    MSIZE_WORD,     /* 16 bits */
    MSIZE_DWORD,    /* 32 bits */
    MSIZE_QWORD,    /* 64 bits */
    MSIZE_MAX,
} msize_t;

/*
 * Convert a program data type to a machine size
 * constant.
 *
 * @type: Type to convert
 *
 * Returns MSIZE_BAD on failure
 */
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

/*
 * Emit a return instruction as well as loading the return
 * register with an immediate
 *
 * @state: Compiler state
 * @size:  Return size
 * @imm:   Immediate to return
 *
 * Returns zero on success
 */
int mu_cg_retimm(struct gup_state *state, msize_t size, ssize_t imm);

/*
 * Emit a jump to a label
 *
 * @state: Compiler state
 * @s:     Label to jump to
 *
 * Returns zero on success
 */
int mu_cg_jmp(struct gup_state *state, const char *s);

/*
 * Emit a call to a label
 *
 * @state: Compiler state
 * @s:     Label to call
 *
 * Returns zero on success
 */
int mu_cg_call(struct gup_state *state, const char *s);

/*
 * Emit a struct
 *
 * @state:  Compiler state
 * @parent: Parent node
 *
 * Returns zero on success
 */
int mu_cg_struct(struct gup_state *state, struct ast_node *parent);

/*
 * Emit a load to a label
 *
 * @state: Compiler state
 * @label: Label to load with value
 * @size:  Size of value to load
 * @ival:  Immediate value to load
 *
 * Returns zero on success
 */
int mu_cg_loadvar(
    struct gup_state *state, const char *label,
    msize_t size, ssize_t ival
);

/*
 * Emit a variable
 *
 * @state: Compiler state
 * @sect:  Section to put label in
 * @label: Label of variable to create
 * @size:  Variable size
 * @ival:  Initial value
 *
 * Returns zero on success
 */
int mu_cg_var(
    struct gup_state *state, bin_section_t sect,
    const char *label, msize_t size, ssize_t ival
);

#endif  /* !GUP_MU_H */
