#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "gup/symbol.h"

int
symbol_table_init(struct symbol_table *symtab)
{
    if (symtab == NULL) {
        errno = -EINVAL;
        return -1;
    }

    symtab->symbol_count = 0;
    TAILQ_INIT(&symtab->symbols);
    return 0;
}

int
symbol_new(struct symbol_table *symtab, const char *name, gup_type_t type,
    struct symbol **res)
{
    struct symbol *symbol;
    struct datum_type *dtype;

    if (symtab == NULL || name == NULL) {
        errno = -EINVAL;
        return -1;
    }

    symbol = malloc(sizeof(*symbol));
    if (symbol == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    /* Initialize the symbol */
    memset(symbol, 0, sizeof(*symbol));
    symbol->id = symtab->symbol_count++;
    symbol->name = strdup(name);

    /* Initialize the datam type */
    dtype = &symbol->data_type;
    dtype->type = type;

    if (res != NULL) {
        *res = symbol;
    }

    TAILQ_INSERT_TAIL(&symtab->symbols, symbol, link);
    return 0;
}

struct symbol *
symbol_from_id(struct symbol_table *symtab, sym_id_t id)
{
    struct symbol *symbol;

    if (symtab == NULL) {
        return NULL;
    }

    TAILQ_FOREACH(symbol, &symtab->symbols, link) {
        if (symbol->id == id) {
            return symbol;
        }
    }

    return NULL;
}

void
symbol_table_destroy(struct symbol_table *symtab)
{
    struct symbol *symbol;

    if (symtab == NULL) {
        return;
    }

    symbol = TAILQ_FIRST(&symtab->symbols);
    while (symbol != NULL) {
        TAILQ_REMOVE(&symtab->symbols, symbol, link);
        if (symbol->name != NULL) {
            free(symbol->name);
        }
        free(symbol);
        symbol = TAILQ_FIRST(&symtab->symbols);
    }
}
