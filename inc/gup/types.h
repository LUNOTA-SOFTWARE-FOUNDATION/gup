/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_TYPE_H
#define GUP_TYPE_H 1

/*
 * Represents valid program types besides
 * GUP_TYPE_BAD
 */
typedef enum {
    GUP_TYPE_BAD,
    GUP_TYPE_VOID,
    GUP_TYPE_U8,
    GUP_TYPE_U16,
    GUP_TYPE_U32,
    GUP_TYPE_U64
} gup_type_t;

/*
 * Represents the specific type of a piece of data
 *
 * @type: Type of data
 * @ptr_depth: Pointer depth (0 if non-pointer)
 */
struct datum_type {
    gup_type_t type;
    size_t ptr_depth;
};

#endif  /* !GUP_TYPE_H */
