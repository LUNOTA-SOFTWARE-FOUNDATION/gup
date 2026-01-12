/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "gup/state.h"
#include "gup/parser.h"

#define GUP_VERSION "0.0.3"
#define ELAPSED_NS(STARTP, ENDP)                            \
    (double)((ENDP)->tv_sec - (STARTP)->tv_sec) * 1.0e9 +    \
        (double)((ENDP)->tv_nsec - (STARTP)->tv_nsec)

static bool asm_only = false;
static const char *bin_fmt = "elf64";

static void
help(void)
{
    printf(
        "the gup compiler - gup!\n"
        "-----------------------------\n"
        "[-h]   Display this help menu\n"
        "[-v]   Display the version\n"
        "[-a]   Assembly output only\n"
        "[-f]   Output format\n"
        "...... [elf64]\n"
        "...... [bin]\n"
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

static void
assemble(const char *path)
{
    char cmd[64];

    snprintf(
        cmd,
        sizeof(cmd),
        "nasm -f%s %s",
        bin_fmt,
        path
    );

    system(cmd);
    remove(path);
}

static int
compile(const char *path)
{
    struct gup_state state;
    struct timespec start, end;
    double elapsed_ms, elapsed_ns;

    if (path == NULL) {
        return -1;
    }

    if (gup_state_init(path, &state) < 0) {
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &start);
    if (gup_parse(&state) < 0) {
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    elapsed_ns = ELAPSED_NS(&start, &end);
    elapsed_ms = elapsed_ns / 1e+6;
    printf("compiled in %.2fms [%.2fns]\n", elapsed_ms, elapsed_ns);

    gup_state_destroy(&state);
    if (!asm_only) {
        assemble(DEFAULT_ASMOUT);
    }

    return 0;
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

    while ((opt = getopt(argc, argv, "hvaf:")) != -1) {
        switch (opt) {
        case 'h':
            help();
            return -1;
        case 'v':
            version();
            return -1;
        case 'a':
            asm_only = true;
            break;
        case 'f':
            bin_fmt = strdup(optarg);
            break;
        }
    }

    while (optind < argc) {
        if (compile(argv[optind++]) < 0) {
            return -1;
        }
    }

    return 0;
}
