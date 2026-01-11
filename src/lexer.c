/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gup/lexer.h"
#include "gup/trace.h"

/*
 * Place a byte in the putback buffer
 *
 * @state: Compiler state
 * @c: Byte to place
 */
static inline void
lexer_putback(struct gup_state *state, char c)
{
    if (state == NULL) {
        return;
    }

    state->putback = c;
}

/*
 * Returns true if the input character 'c' is
 * a whitespace character
 *
 * @state: Compiler state
 * @c: Character to check against
 */
static bool
lexer_is_ws(struct gup_state *state, char c)
{
    if (state == NULL) {
        return false;
    }

    switch (c) {
    case '\n':
        ++state->line_num;
    case '\r':
    case '\f':
    case '\t':
    case ' ':
        return true;
    }

    return false;
}

/*
 * Consume a single byte from the input source file
 *
 * @state: Compiler state
 * @accept_ws: If true, accept whitespace
 *
 * Returns the character grabbed from the input source
 */
static char
lexer_nom(struct gup_state *state, bool accept_ws)
{
    char c;

    if (state == NULL) {
        return '\0';
    }

    /* Take from the putback buffer if we can */
    if (state->putback != '\0') {
        c = state->putback;
        state->putback = '\0';
        if (lexer_is_ws(state, c) && accept_ws)
            return c;
        if (!lexer_is_ws(state, c))
            return c;
    }

    while (read(state->in_fd, &c, 1) > 0) {
        if (lexer_is_ws(state, c) && !accept_ws) {
            continue;
        }

        return c;
    }

    return '\0';
}

/*
 * Scan a line of assembly
 *
 * @state: Compiler state
 * @res: Token result written here
 *
 * Returns zero on success
 */
static int
lexer_scan_asm(struct gup_state *state, struct token *res)
{
    char *buf;
    size_t buf_cap;
    size_t buf_size;
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    buf_cap = 8;
    buf_size = 0;
    if ((buf = malloc(buf_cap)) == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    /*
     * This serves to ensure the assembly output stays
     * pretty without any weird whitespaces. If the
     * programmer skipped a whitespace after the '@',
     * put whatever we grabbed back.
     */
    if ((c = lexer_nom(state, true)) != ' ') {
        lexer_putback(state, c);
    }

    for (;;) {
        if ((c = lexer_nom(state, true)) == '\0') {
            trace_error(state, "unexpected end of file\n");
            trace_warn("missing a semicolon?\n");
            free(buf);
            return -1;
        }

        /* Is this the end of the assembly? */
        if (c == ';') {
            buf[buf_size] = '\0';
            break;
        }

        buf[buf_size++] = c;
        if (buf_size >= buf_cap - 1) {
            buf_cap += 8;
            buf = realloc(buf, buf_cap);
        }

        if (buf == NULL) {
            return -1;
        }
    }

    res->s = ptrbox_strdup(&state->ptrbox, buf);
    res->type = TT_ASM;
    free(buf);
    return 0;
}

/*
 * Scan a number in the source input
 *
 * @state: Compiler state
 * @lc: Last character
 * @res: Token result is written here
 *
 * Returns zero on success
 */
static int
lexer_scan_num(struct gup_state *state, int lc, struct token *res)
{
    char buf[22];
    uint8_t buf_i = 0;
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    buf[buf_i++] = lc;
    for (;;) {
        if ((c = lexer_nom(state, false)) == '\0') {
            buf[buf_i] = '\0';
            break;
        }

        /*
         * Sometimes large numbers may be hard to read, the '_'
         * character is valid to seperate digits and serves no
         * programmatic purpose.
         */
        if (c == '_') {
            continue;
        }

        if (!isdigit(c)) {
            buf[buf_i] = '\0';
            lexer_putback(state, c);
            break;
        }

        buf[buf_i++] = c;
        if (buf_i >= sizeof(buf) - 1) {
            buf[buf_i] = '\0';
            break;
        }
    }

    res->v = atoi(buf);
    res->type = TT_NUMBER;
    return 0;
}

/*
 * Scan an identifier from the source input
 *
 * @state: Compiler state
 * @lc: Last character
 * @res: Token result is written here
 *
 * Returns zero on success
 */
static int
lexer_scan_ident(struct gup_state *state, int lc, struct token *res)
{
    char *buf;
    size_t buf_cap;
    size_t buf_size;
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    buf_cap = 8;
    buf_size = 0;
    if ((buf = malloc(buf_cap)) == NULL) {
        errno = -ENOMEM;
        return -1;
    }

    buf[buf_size++] = lc;
    for (;;) {
        if ((c = lexer_nom(state, true)) == '\0') {
            buf[buf_size] = '\0';
            break;
        }

        if (!isalnum(c) && c != '_') {
            lexer_putback(state, c);
            buf[buf_size] = '\0';
            break;
        }

        buf[buf_size++] = c;
        if (buf_size >= buf_cap - 1) {
            buf_cap += 8;
            buf = realloc(buf, buf_cap);
        }

        if (buf == NULL) {
            errno = -ENOMEM;
            return -1;
        }
    }

    res->type = TT_IDENT;
    res->s = ptrbox_strdup(&state->ptrbox, buf);
    free(buf);
    return 0;
}

/*
 * Check if what was scanned as an identifier is actually
 * a keyword.
 *
 * @state: Compiler state
 * @tok: Token to check
 *
 * Returns zero on success
 */
static int
lexer_is_kw(struct gup_state *state, struct token *tok)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_IDENT) {
        errno = -EINVAL;
        return -1;
    }

    switch (*tok->s) {
    case 'u':
        if (strcmp(tok->s, "u8") == 0) {
            tok->type = TT_U8;
            return 0;
        }

        if (strcmp(tok->s, "u16") == 0) {
            tok->type = TT_U16;
            return 0;
        }

        if (strcmp(tok->s, "u32") == 0) {
            tok->type = TT_U32;
            return 0;
        }

        if (strcmp(tok->s, "u64") == 0) {
            tok->type = TT_U64;
            return 0;
        }

        break;
    case 'p':
        if (strcmp(tok->s, "proc") == 0) {
            tok->type = TT_PROC;
            return 0;
        }

        if (strcmp(tok->s, "pub") == 0) {
            tok->type = TT_PUB;
            return 0;
        }

        break;
    }

    return -1;
}

int
lexer_scan(struct gup_state *state, struct token *res)
{
    char c;

    if (state == NULL || res == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /* Consume a single byte */
    if ((c = lexer_nom(state, false)) == '\0') {
        return -1;
    }

    switch (c) {
    case '@':
        if (lexer_scan_asm(state, res) < 0) {
            return -1;
        }

        return 0;
    case ';':
        res->type = TT_SEMI;
        res->c = c;
        return 0;
    case '*':
        res->type = TT_STAR;
        res->c = c;
        return 0;
    case '+':
        res->type = TT_PLUS;
        res->c = c;
        return 0;
    case '-':
        res->type = TT_MINUS;
        res->c = c;
        return 0;
    case '/':
        res->type = TT_SLASH;
        res->c = c;
        return 0;
    case '(':
        res->type = TT_LPAREN;
        res->c = c;
        return 0;
    case ')':
        res->type = TT_RPAREN;
        res->c = c;
        return 0;
    case '<':
        res->type = TT_LT;
        res->c = c;
        return 0;
    case '>':
        res->type = TT_GT;
        res->c = c;
        return 0;
    case '{':
        res->type = TT_LBRACE;
        res->c = c;
        return 0;
    case '}':
        res->type = TT_RBRACE;
        res->c = c;
        return 0;
    default:
        /* Is this a digit? */
        if (isdigit(c)) {
            if (lexer_scan_num(state, c, res) < 0)
                return -1;

            return 0;
        }

        /* Is this an identifier? */
        if (lexer_scan_ident(state, c, res) == 0) {
            lexer_is_kw(state, res);
            return 0;
        }
    }

    return -1;
}
