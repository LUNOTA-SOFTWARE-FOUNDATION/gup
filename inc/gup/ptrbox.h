/*
 * Copyright (C) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#ifndef GUP_PTRBOX_H
#define GUP_PTRBOX_H

#include <sys/queue.h>
#include <stdint.h>
#include <stddef.h>

/*
 * Represents a pointer box entry which stores a reference
 * to allocated memory
 *
 * @data: Allocated memory
 * @link: Queue link
 */
struct ptrbox_entry {
    void *data;
    TAILQ_ENTRY(ptrbox_entry) link;
};

/*
 * A pointer box stores one or more references to allocated memory
 * so that it can be cleaned up in one sweep when usage is complete.
 *
 * @entries: Pointer box entries
 * @entry_count: Number of entries in pointer box
 */
struct ptrbox {
    TAILQ_HEAD(, ptrbox_entry) entries;
    size_t entry_count;
};

/*
 * Allocate memory and save it in a pointer box
 *
 * @ptrbox: Pointer box to save reference in
 * @sz: Allocation size
 *
 * Returns the base of the allocated memory on success
 */
void *ptrbox_alloc(struct ptrbox *ptrbox, size_t sz);

/*
 * Perform a strdup() operation and save the reference in
 * a pointer box
 *
 * @ptrbox: Ptrbox to save reference within
 * @s: String to dup
 *
 * Returns dupped string on success
 */
void *ptrbox_strdup(struct ptrbox *ptrbox, const char *s);

/*
 * Destroy a pointer box
 *
 * @ptrbox: Pointer box to destory
 */
void ptrbox_destroy(struct ptrbox *ptrbox);

/*
 * Initialize a pointer box
 *
 * @res: Pointer box to initialize
 *
 * Returns zero on success
 */
int ptrbox_init(struct ptrbox *res);

#endif  /* !GUP_PTRBOX_H */
