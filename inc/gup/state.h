/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_STATE_H
#define GUP_STATE_H

#include <stdio.h>
#include "gup/ptrbox.h"
#include "gup/token.h"
#include "gup/symbol.h"

#define DEFAULT_ASMOUT "gupgen.asm"
#define MAX_SCOPE_DEPTH 8

/*
 * Represents valid sections within the output
 * binary
 */
typedef enum {
    SECTION_NONE,
    SECTION_TEXT,
    SECTION_DATA,
    SECTION_BSS,
    SECTION_MAX
} bin_section_t;

/*
 * Represents the compiler state
 *
 * @in_fd: Source input file descriptor
 * @line_no: Line number
 * @putback: Putback buffer
 * @ptrbox: Global pointer box
 * @symtab: Global symbol table
 * @scope_stack: Keeps track of scopes
 * @scope_depth: Current scope depth
 * @loop_count: Number of loops in program
 * @cur_section: Current section
 * @out_fp: Output file
 */
struct gup_state {
    int in_fd;
    size_t line_num;
    char putback;
    struct ptrbox ptrbox;
    struct symbol_table symtab;
    tt_t scope_stack[MAX_SCOPE_DEPTH];
    size_t scope_depth;
    size_t loop_count;
    bin_section_t cur_section;
    FILE *out_fp;
};

/*
 * Initialize the compiler state
 *
 * @path: Source input file path
 * @state: Compiler state
 *
 * Returns zero on success
 */
int gup_state_init(const char *path, struct gup_state *state);

/*
 * Destroy the compiler state
 *
 * @state: Compiler state the destroy
 */
void gup_state_destroy(struct gup_state *state);

#endif  /* !GUP_STATE_H */
