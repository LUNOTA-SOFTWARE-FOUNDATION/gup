/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <errno.h>
#include "gup/mu.h"
#include "gup/state.h"

static const char *sectab[] = {
    [SECTION_NONE] = "none",
    [SECTION_TEXT] = ".text",
    [SECTION_DATA] = ".data",
    [SECTION_BSS]  = ".bss"
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
