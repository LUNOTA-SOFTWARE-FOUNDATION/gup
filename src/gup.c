/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <unistd.h>

#define GUP_VERSION "0.0.1"

static void
help(void)
{
    printf(
        "the gup compiler - gup!\n"
        "-----------------------------\n"
        "[-h]   Display this help menu\n"
        "[-v]   Display the version\n"
    );
}

static void
version(void)
{
    printf(
        "------------------------------\n"
        "gup compiler -- v%s\n"
        "Copyright (c) 2026 Ian Moffett\n"
        "------------------------------\n",
        GUP_VERSION
    );
}

int
main(int argc, char **argv)
{
    int opt;

    if (argc < 2) {
        printf("fatal: too few arguments!\n");
        help();
        return -1;
    }

    while ((opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
        case 'h':
            help();
            return -1;
        case 'v':
            version();
            return -1;
        }
    }
    return 0;
}
