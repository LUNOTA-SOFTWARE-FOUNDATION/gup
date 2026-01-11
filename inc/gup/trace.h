/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_TRACE_H
#define GUP_TRACE_H 1

#include <stdio.h>
#include "gup/state.h"

#define trace_error(gup_state, fmt, ...)    \
    printf("[error]: " fmt, ##__VA_ARGS__); \
    printf("[near line %zu]\n", (gup_state)->line_num);
#define trace_warn(fmt, ...)   \
    printf("[warn]: " fmt, ##__VA_ARGS__)

#define DEBUG 1
#if DEBUG
#define trace_debug(fmt, ...)   \
    printf("[debug]: " fmt, ##__VA_ARGS__)
#else
#define trace_debug(...) (void)0
#endif  /* DEBUG */

#endif  /* !GUP_TRACE_H */
