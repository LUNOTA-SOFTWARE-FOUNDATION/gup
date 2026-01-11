/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_MU_H
#define GUP_MU_H 1

#include <stdbool.h>
#include "gup/state.h"

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
 * Emit a jump to a label
 *
 * @state: Compiler state
 * @s:     Label to jump to
 *
 * Returns zero on success
 */
int mu_cg_jmp(struct gup_state *state, const char *s);

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
