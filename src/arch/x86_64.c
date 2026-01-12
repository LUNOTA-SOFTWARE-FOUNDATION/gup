/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "gup/mu.h"
#include "gup/state.h"
#include "gup/trace.h"

/* Section lookup table */
static const char *sectab[] = {
    [SECTION_NONE] = "none",
    [SECTION_TEXT] = ".text",
    [SECTION_DATA] = ".data",
    [SECTION_BSS]  = ".bss"
};

/* Define <n> size lookup table */
static const char *dsztab[] = {
    [MSIZE_BAD]  = "bad",
    [MSIZE_BYTE] = "db",
    [MSIZE_WORD] = "dw",
    [MSIZE_DWORD] = "dd",
    [MSIZE_QWORD] = "dq"
};

/* Return <n> size lookup table */
static const char *rettab[] = {
    [MSIZE_BAD] = "bad",
    [MSIZE_BYTE] = "al",
    [MSIZE_WORD] = "ax",
    [MSIZE_DWORD] = "eax",
    [MSIZE_QWORD] = "rax"
};

/*
 * Ensure that we are currently in the desired section
 *
 * @state: Compiler state
 * @what:  Expected section
 */
static void
cg_assert_section(struct gup_state *state, bin_section_t what)
{
    if (state == NULL) {
        return;
    }

    if (state->cur_section != what) {
        fprintf(
            state->out_fp,
            "[section %s]\n",
            sectab[what]
        );

        state->cur_section = what;
    }
}

int
mu_cg_inject(struct gup_state *state, const char *str)
{
    if (state == NULL || str == NULL) {
        errno = -EINVAL;
        return -1;
    }

    cg_assert_section(state, SECTION_TEXT);
    fprintf(
        state->out_fp,
        "\t%s\n",
        str
    );
    return 0;
}

int
mu_cg_label(struct gup_state *state, const char *s, bool is_global)
{
    if (state == NULL || s == NULL) {
        errno = -EINVAL;
        return -1;
    }

    cg_assert_section(state, SECTION_TEXT);
    if (is_global) {
        fprintf(
            state->out_fp,
            "[global %s]\n",
            s
        );
    }

    fprintf(
        state->out_fp,
        "%s:\n",
        s
    );

    return 0;
}

int
mu_cg_ret(struct gup_state *state)
{
    if (state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    fprintf(
        state->out_fp,
        "\tret\n"
    );

    return 0;
}

int
mu_cg_jmp(struct gup_state *state, const char *s)
{
    if (state == NULL || s == NULL) {
        errno = -EINVAL;
        return -1;
    }

    fprintf(
        state->out_fp,
        "\tjmp %s\n", s
    );

    return 0;
}

int
mu_cg_var(struct gup_state *state, bin_section_t sect, const char *label,
    msize_t size, ssize_t ival)
{
    if (state == NULL || label == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (size >= MSIZE_MAX) {
        errno = -EINVAL;
        return -1;
    }

    if (sect >= SECTION_MAX) {
        errno = -EINVAL;
        return -1;
    }

    cg_assert_section(state, sect);
    fprintf(
        state->out_fp,
        "%s: %s %zd\n",
        label,
        dsztab[size],
        ival
    );

    return 0;
}

int
mu_cg_call(struct gup_state *state, const char *s)
{
    if (state == NULL || s == NULL) {
        errno = -EINVAL;
        return -1;
    }

    fprintf(
        state->out_fp,
        "\tcall %s\n",
        s
    );

    return 0;
}

int
mu_cg_retimm(struct gup_state *state, msize_t size, ssize_t imm)
{
    if (state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (size >= MSIZE_MAX) {
        errno = -EINVAL;
        return -1;
    }

    fprintf(
        state->out_fp,
        "\tmov %s, %zd\n"
        "\tret\n",
        rettab[size],
        imm
    );

    return 0;
}

int
mu_cg_struct(struct gup_state *state, struct ast_node *parent)
{
    struct ast_node *cur;
    msize_t size;

    if (state == NULL || parent == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (parent->type != AST_STRUCT) {
        trace_error(
            state,
            "expected AST_STRUCT got %d\n",
            parent->type
        );

        return -1;
    }

    cg_assert_section(state, SECTION_DATA);
    cur = parent->right->right;

    while (cur != NULL) {
        size = type_to_msize(cur->field_type);
        if (size == MSIZE_BAD) {
            cur = cur->right;
            continue;
        }

        fprintf(
            state->out_fp,
            "%s.%s: %s 0\n",
            parent->s,
            cur->s,
            dsztab[size]
        );

        cur = cur->right;
    }

    return 0;
}
