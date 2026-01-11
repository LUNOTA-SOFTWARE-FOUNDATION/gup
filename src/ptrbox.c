#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "gup/ptrbox.h"

int
ptrbox_init(struct ptrbox *res)
{
    if (res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    TAILQ_INIT(&res->entries);
    res->entry_count = 0;
    return 0;
}

void *
ptrbox_alloc(struct ptrbox *ptrbox, size_t sz)
{
    struct ptrbox_entry *entry;

    if (ptrbox == NULL || sz == 0) {
        return NULL;
    }

    if ((entry = malloc(sizeof(*entry))) == NULL) {
        return NULL;
    }

    if ((entry->data = malloc(sz)) == NULL) {
        free(entry);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&ptrbox->entries, entry, link);
    ++ptrbox->entry_count;
    return entry->data;
}

void *
ptrbox_strdup(struct ptrbox *ptrbox, const char *s)
{
    struct ptrbox_entry *entry;

    if (ptrbox == NULL || s == 0) {
        return NULL;
    }

    if ((entry = malloc(sizeof(*entry))) == NULL) {
        return NULL;
    }

    if ((entry->data = strdup(s)) == NULL) {
        free(entry);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&ptrbox->entries, entry, link);
    ++ptrbox->entry_count;
    return entry->data;
}

void
ptrbox_destroy(struct ptrbox *ptrbox)
{
    struct ptrbox_entry *entry;

    if (ptrbox == NULL) {
        return;
    }

    if ((entry = TAILQ_FIRST(&ptrbox->entries)) == NULL) {
        return;
    }

    do {
        TAILQ_REMOVE(&ptrbox->entries, entry, link);
        free(entry->data);
        free(entry);
        entry = TAILQ_FIRST(&ptrbox->entries);
    } while (entry != NULL);
}
