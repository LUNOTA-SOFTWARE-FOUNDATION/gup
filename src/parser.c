/*
 * Copyright (c) 2026, Ian Moffett.
 * Provided under the BSD-3 clause.
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "gup/parser.h"
#include "gup/token.h"
#include "gup/lexer.h"
#include "gup/trace.h"
#include "gup/ast.h"
#include "gup/codegen.h"
#include "gup/types.h"
#include "gup/scope.h"

#define tokstr(tt)          \
    toktab[(tt)]

#define tokstr1(tok)        \
    tokstr((tok)->type)

#define utok(state, tt)          \
    trace_error(                 \
        state,                   \
        "unexpected token %s\n", \
        tokstr(tt)               \
    );

#define utok1(state, exp, got)           \
    trace_error(                         \
        state,                           \
        "expected %s, got %s instead\n", \
        exp,                             \
        got                              \
    );

#define ueof(state)                 \
    trace_error(                    \
        state,                      \
        "unexpected end of file\n"  \
    );

/* Most previous input token */
static struct token last_token;
static struct token tail_token; /* Previous previous token */

/*
 * A lookup table used to convert token constants
 * to human readable strings.
 */
static const char *toktab[] = {
    [TT_NONE]   = "NONE",
    [TT_ASM]    = "ASM",
    [TT_SEMI]   = "SEMICOLON",
    [TT_STAR]   = "STAR",
    [TT_PLUS]   = "PLUS",
    [TT_MINUS]  = "MINUS",
    [TT_SLASH]  = "SLASH",
    [TT_LPAREN] = "LPAREN",
    [TT_RPAREN] = "RPAREN",
    [TT_LBRACE] = "LBRACE",
    [TT_RBRACE] = "RBRACE",
    [TT_LT]     = "LESS-THAN",
    [TT_GT]     = "GREATER-THAN",
    [TT_DOT]    = "DOT",
    [TT_U8]     = "U8",
    [TT_U16]    = "U16",
    [TT_U32]    = "U32",
    [TT_U64]    = "U64",
    [TT_VOID]   = "VOID",
    [TT_PUB]    = "PUB",
    [TT_PROC]   = "PROC",
    [TT_LOOP]   = "LOOP",
    [TT_BREAK]  = "BREAK",
    [TT_CONT]   = "CONTINUE",
    [TT_RETURN] = "RETURN",
    [TT_STRUCT] = "STRUCT",
    [TT_NUMBER] = "NUMBER",
    [TT_IDENT]  = "IDENTIFIER",
    [TT_COMMENT] = "COMMENT"
};

/*
 * Lookbehind current token
 *
 * @n: Number of steps to look behind
 * @tok: Result is written here
 *
 * XXX: 'n' being zero returns the current token
 *
 * TODO: Use a token buffer
 */
static inline int
parse_lookbehind(size_t n, struct token *tok)
{
    if (n == 0) {
        *tok = last_token;
        return 0;
    }

    if (n > 1) {
        return -1;
    }

    *tok = tail_token;
    return 0;
}

/*
 * Get a data type from a lexical token type
 *
 * @tt: Token type to test
 *
 * Returns GUP_TYPE_BAD on failure, otherwise the
 * specific data type.
 */
static gup_type_t
parse_get_type(tt_t tt)
{
    switch (tt) {
    case TT_VOID: return GUP_TYPE_VOID;
    case TT_U8:   return GUP_TYPE_U8;
    case TT_U16:  return GUP_TYPE_U16;
    case TT_U32:  return GUP_TYPE_U32;
    case TT_U64:  return GUP_TYPE_U64;
    default:      return GUP_TYPE_BAD;
    }

    return GUP_TYPE_BAD;
}

/*
 * If we are currently in a loop, return true,
 * otherwise false.
 *
 * @state: Compiler state
 */
static bool
parse_in_loop(struct gup_state *state)
{
    tt_t scope;

    if (state == NULL) {
        return false;
    }

    scope = scope_top(state);
    switch (scope) {
    case TT_LOOP:
        return true;
    default:
        return false;
    }

    return false;
}

/*
 * Parse a pointer
 *
 * @state: Compiler state
 * @tok:   Last token
 * @datum: Type pointer belongs to
 *
 * Returns zero on success
 */
static int
parse_ptr(struct gup_state *state, struct token *tok, struct datum_type *datum)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (datum == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_STAR) {
        utok1(state, "STAR", tokstr1(tok));
        return -1;
    }

    while (tok->type == TT_STAR) {
        if (lexer_scan(state, tok) < 0) {
            ueof(state);
            return -1;
        }
        ++datum->ptr_depth;
    }

    return 0;
}

/*
 * Parse a data type
 *
 * @state: Compiler state
 * @tok:   Last token
 * @res:   Type result written here
 *
 * Returns zero on success
 */
static int
parse_type(struct gup_state *state, struct token *tok, struct datum_type *res)
{
    gup_type_t type;

    type = parse_get_type(tok->type);
    if (type == GUP_TYPE_BAD) {
        utok1(state, "TYPE", tokstr1(tok));
        return -1;
    }

    res->type = type;
    res->ptr_depth = 0;

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if (tok->type == TT_STAR) {
        if (parse_ptr(state, tok, res) < 0)
            return -1;
    }

    return 0;
}

/*
 * Asserts that the next token is of an expected value
 *
 * @state: Compiler state
 * @tok    Last token
 * @what:  Expected token
 *
 * Returns zero on success
 */
static int
parse_expect(struct gup_state *state, struct token *tok, tt_t what)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    /* This must match */
    if (tok->type != what) {
        utok1(state, tokstr(what), tokstr1(tok));
        return -1;
    }

    return 0;
}

/*
 * Handle lines of assembly
 *
 * @state: Compiler state
 * @tok: Last token
 *
 * Returns zero on success
 */
static int
parse_asm(struct gup_state *state, struct token *tok)
{
    struct ast_node *node;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (ast_alloc_node(state, AST_ASM, &node) < 0) {
        trace_error(state, "failed to allocate AST_ASM\n");
        return -1;
    }

    node->s = tok->s;
    return cg_compile_node(state, node);
}

/*
 * Handle for when we encounter a right brace ('}')
 *
 * @state: Compiler state
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_rbrace(struct gup_state *state, struct token *tok)
{
    struct ast_node *root;
    tt_t scope;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((scope = scope_pop(state)) == TT_NONE) {
        trace_error(state, "unexpected RBRACE, no previous scope\n");
        return -1;
    }

    switch (scope) {
    case TT_PROC:
        if (state->unreachable) {
            state->unreachable = 0;
            return 0;
        }

        if (ast_alloc_node(state, AST_PROC, &root) < 0) {
            trace_error(state, "could not allocate AST_PROC epilogue\n");
            return -1;
        }

        root->epilogue = 1;
        state->this_func = NULL;
        return cg_compile_node(state, root);
    case TT_LOOP:
        if (ast_alloc_node(state, AST_LOOP, &root) < 0) {
            trace_error(state, "could not allocate AST_LOOP epilogue\n");
            return -1;
        }

        root->epilogue = 1;
        return cg_compile_node(state, root);
    default:
        break;
    }

    return 0;
}

/*
 * Handle for when we encounter a left brace ('{')
 *
 * @state: Compiler state
 * @block: Parent block token
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_lbrace(struct gup_state *state, tt_t block, struct token *tok)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (scope_push(state, block) < 0) {
        return -1;
    }

    return 0;
}

/*
 * Parse a procedure
 *
 * @state: Compiler state
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_proc(struct gup_state *state, struct token *tok)
{
    struct ast_node *root;
    struct token prev_tok;
    struct datum_type type;
    struct symbol *symbol;
    bool is_global = false;
    int error;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (state->this_func != NULL) {
        trace_error(state, "nested functions not supported\n");
        return -1;
    }

    if (parse_lookbehind(1, &prev_tok) < 0) {
        trace_error(state, "lookbehind failure\n");
        return -1;
    }

    if (prev_tok.type == TT_PUB) {
        is_global = true;
    }

    if (parse_expect(state, tok, TT_IDENT) < 0) {
        return -1;
    }

    if (ast_alloc_node(state, AST_PROC, &root) < 0) {
        return -1;
    }

    /* Duplicate the identifier */
    root->s = ptrbox_strdup(&state->ptrbox, tok->s);
    if (root->s == NULL) {
        return -1;
    }

    if (parse_expect(state, tok, TT_MINUS) < 0) {
        return -1;
    }

    if (parse_expect(state, tok, TT_GT) < 0) {
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    if (parse_type(state, tok, &type) < 0) {
        return -1;
    }

    error = symbol_new(
        &state->symtab,
        root->s,
        type.type,
        &symbol
    );

    if (error < 0) {
        trace_error(state, "failed to create new symbol\n");
        return -1;
    }

    /* Set the new symbol */
    symbol->global = is_global;
    symbol->type = SYMBOL_FUNC;
    symbol->data_type = type;
    root->symbol = symbol;

    switch (tok->type) {
    case TT_SEMI:
        return 0;
    case TT_LBRACE:
        if (parse_lbrace(state, TT_PROC, tok) < 0) {
            return -1;
        }

        state->this_func = symbol;
        return cg_compile_node(state, root);
    default:
        utok(state, tok->type);
        return -1;
    }

    return -1;
}

static int
parse_loop(struct gup_state *state, struct token *tok)
{
    struct ast_node *root;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (parse_expect(state, tok, TT_LBRACE) < 0) {
        return -1;
    }

    if (scope_push(state, TT_LOOP) < 0) {
        return -1;
    }

    if (ast_alloc_node(state, AST_LOOP, &root) < 0) {
        trace_error(state, "failed to allocate AST_LOOP\n");
        return -1;
    }

    return cg_compile_node(state, root);
}

/*
 * Parse a variable
 *
 * @state: Compiler state
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_var(struct gup_state *state, struct token *tok)
{
    struct datum_type type;
    struct symbol *symbol;
    struct ast_node *root;
    int error;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    /* TODO: Add support for local variables */
    if (scope_top(state) != TT_NONE) {
        trace_error(state, "only globals are supported now\n");
        return -1;
    }

    /* We need a type */
    if (parse_type(state, tok, &type) < 0) {
        return -1;
    }

    /* Now an identifier */
    if (tok->type != TT_IDENT) {
        utok1(state, "IDENT", tokstr1(tok));
        return -1;
    }

    error = symbol_new(
        &state->symtab,
        tok->s,
        type.type,
        &symbol
    );

    if (error < 0) {
        trace_error(state, "failed to create symbol\n");
        return -1;
    }

    if (ast_alloc_node(state, AST_GLOBVAR, &root) < 0) {
        trace_error(state, "failed to allocate AST_GLOBVAR\n");
        return -1;
    }

    symbol->type = SYMBOL_VAR;
    symbol->data_type = type;
    root->symbol = symbol;

    if (parse_expect(state, tok, TT_SEMI) < 0) {
        return -1;
    }

    return cg_compile_node(state, root);
}

/*
 * Parse a break statement
 *
 * @state: Compiler state
 * @tok: Last token
 */
static int
parse_break(struct gup_state *state, struct token *tok)
{
    struct ast_node *node;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (!parse_in_loop(state)) {
        trace_error(state, "break statement not in a loop\n");
        return -1;
    }

    if (parse_expect(state, tok, TT_SEMI) < 0) {
        return -1;
    }

    if (ast_alloc_node(state, AST_BREAK, &node) < 0) {
        trace_error(state, "failed to allocate AST_BREAK\n");
        return -1;
    }

    return cg_compile_node(state, node);
}

/*
 * Parse a function call
 *
 * @state: Compiler state
 * @ident: Identifier
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_call(struct gup_state *state, const char *ident, struct token *tok)
{
    struct symbol *symbol;
    struct ast_node *root;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_LPAREN) {
        utok1(state, "LPAREN", tokstr1(tok));
        return -1;
    }

    /* TODO: Handle arguments */
    if (parse_expect(state, tok, TT_RPAREN) < 0) {
        return -1;
    }

    symbol = symbol_from_name(&state->symtab, ident);
    if (symbol == NULL) {
        trace_error(state, "undefined reference to function %s\n", ident);
        return -1;
    }

    if (ast_alloc_node(state, AST_CALL, &root) < 0) {
        trace_error(state, "failed to allocate AST_CALL\n");
        return -1;
    }

    if (parse_expect(state, tok, TT_SEMI) < 0) {
        return -1;
    }

    root->symbol = symbol;
    return cg_compile_node(state, root);
}

static int
parse_struct_access(struct gup_state *state, char *ident, struct token *tok)
{
    struct ast_node *root, *cur;

    if (state == NULL || ident == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_DOT) {
        utok1(state, "DOT", tokstr1(tok));
        return -1;
    }

    if (ast_alloc_node(state, AST_ACCESS, &root) < 0) {
        trace_error(state, "failed to allocated AST_ACCESS\n");
        return -1;
    }

    root->s = ident;
    cur = root;

    /* Begin scanning fields */
    for (;;) {
        if (parse_expect(state, tok, TT_IDENT) < 0) {
            return -1;
        }

        if (ast_alloc_node(state, AST_ACCESS, &cur->right) < 0) {
            trace_error(state, "failed to access AST_ACCESS\n");
            return -1;
        }

        cur = cur->right;
        cur->s = ptrbox_strdup(&state->ptrbox, tok->s);
        if (cur->s == NULL) {
            trace_error(state, "failed to dup field name\n");
            return -1;
        }

        /* Next token should be '.' or ';' */
        if (lexer_scan(state, tok) < 0) {
            ueof(state);
            return -1;
        }

        if (tok->type == TT_SEMI) {
            break;
        }

        if (tok->type != TT_DOT) {
            utok1(state, "DOT or SEMI", tokstr1(tok));
            return -1;
        }
    }

    return cg_compile_node(state, root);
}

/*
 * Parse an identifier token
 *
 * @state: Compiler state
 * @tok:   Last token
 *
 * Returns zero on success
 */
static int
parse_ident(struct gup_state *state, struct token *tok)
{
    char *ident;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    ident = ptrbox_strdup(&state->ptrbox, tok->s);
    if (ident == NULL) {
        trace_error(state, "out of memory\n");
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    switch (tok->type) {
    case TT_LPAREN:
        if (parse_call(state, ident, tok) < 0) {
            return -1;
        }

        break;
    case TT_DOT:
        if (parse_struct_access(state, ident, tok) < 0) {
            return -1;
        }

        break;
    default:
        return -1;
    }

    return 0;
}

/*
 * Parse a return statement
 *
 * @state: Compiler state
 * @tok: Last token
 */
static int
parse_return(struct gup_state *state, struct token *tok)
{
    struct ast_node *root;
    struct datum_type *func_type;
    struct symbol *func;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if ((func = state->this_func) == NULL) {
        trace_error(state, "cannot use RETURN outside of function\n");
        return -1;
    }

    if (tok->type != TT_RETURN) {
        utok(state, tok->type);
        return -1;
    }

    func_type = &func->data_type;
    if (func_type->type == GUP_TYPE_VOID) {
        trace_error(state, "cannot use RETURN in VOID function\n");
        return -1;
    }

    /* TODO: Support binary expressions */
    if (parse_expect(state, tok, TT_NUMBER) < 0) {
        return -1;
    }

    if (ast_alloc_node(state, AST_RET, &root) < 0) {
        trace_error(state, "failed to allocate AST_RET\n");
        return -1;
    }

    root->v = tok->v;
    if (parse_expect(state, tok, TT_SEMI) < 0) {
        return -1;
    }

    state->unreachable = 1;
    return cg_compile_node(state, root);
}

/*
 * Parse a struct
 *
 * @state: Compiler state
 * @tok: Last token
 */
static int
parse_struct(struct gup_state *state, struct token *tok)
{
    struct symbol *symbol;
    struct ast_node *root = NULL;
    struct ast_node *cur;
    char *struct_name, *instance_name = NULL;
    char *identifier;
    struct datum_type type;
    int error;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_STRUCT) {
        errno = -EINVAL;
        return -1;
    }

    if (parse_expect(state, tok, TT_IDENT) < 0) {
        return -1;
    }

    struct_name = ptrbox_strdup(&state->ptrbox, tok->s);
    if (struct_name == NULL) {
        trace_error(state, "failed to dup struct name\n");
        return -1;
    }

    if (lexer_scan(state, tok) < 0) {
        ueof(state);
        return -1;
    }

    switch (tok->type) {
    case TT_SEMI:
        return 0;
    case TT_IDENT:
        /*
         * Creating an instance
         *
         * struct <name> <instance_name>;
         */
        instance_name = ptrbox_strdup(&state->ptrbox, tok->s);
        if (instance_name == NULL) {
            trace_error(state, "failed to allocate instance name\n");
            return -1;
        }

        if (parse_expect(state, tok, TT_SEMI) < 0) {
            return -1;
        }

        symbol = symbol_from_name(&state->symtab, struct_name);
        if (ast_alloc_node(state, AST_STRUCT, &cur) < 0) {
            trace_error(state, "failed to allocate AST_STRUCT\n");
            return -1;
        }

        cur->s = ptrbox_strdup(&state->ptrbox, instance_name);
        cur->right = symbol->tree;
        return cg_compile_node(state, cur);
    case TT_LBRACE:
        if (parse_lbrace(state, TT_STRUCT, tok) < 0) {
            return -1;
        }
        break;
    default:
        utok(state, tok->type);
        return -1;
    }

    error = symbol_new(
        &state->symtab,
        struct_name,
        GUP_TYPE_VOID,
        &symbol
    );

    if (error < 0) {
        trace_error(state, "could not create new symbol\n");
        return -1;
    }

    if (ast_alloc_node(state, AST_STRUCT, &root) < 0) {
        trace_error(state, "failed to allocate AST_STRUCT\n");
        return -1;
    }

    cur = root;
    cur->s = struct_name;
    cur->symbol = symbol;

    for (;;) {
        if (lexer_scan(state, tok) < 0) {
            ueof(state);
            return -1;
        }

        if (tok->type == TT_RBRACE) {
            parse_rbrace(state, tok);
            break;
        }

        if (parse_type(state, tok, &type) < 0) {
            return -1;
        }

        if (tok->type != TT_IDENT) {
            utok1(state, "IDENT", tokstr1(tok));
            return -1;
        }

        identifier = ptrbox_strdup(&state->ptrbox, tok->s);
        if (identifier == NULL) {
            trace_error(state, "failed to dup identifier\n");
            return -1;
        }

        if (parse_expect(state, tok, TT_SEMI) < 0) {
            return -1;
        }

        if (ast_alloc_node(state, AST_FIELD, &cur->right) < 0) {
            trace_error(state, "failed to allocate AST_FIELD\n");
            return -1;
        }

        cur = cur->right;
        cur->s = identifier;
        cur->field_type = type.type;
    }

    symbol->tree = root;
    return 0;
}

/*
 * Parse a continue statement
 *
 * @state: Compiler state
 * @tok:   Last token
 *
 * Return zero on success
 */
static int
parse_continue(struct gup_state *state, struct token *tok)
{
    struct ast_node *root;

    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (tok->type != TT_CONT) {
        utok1(state, "CONTINUE", tokstr1(tok));
        return -1;
    }

    if (!parse_in_loop(state)) {
        trace_error(state, "CONTINUE statement not in loop\n");
        return -1;
    }

    if (parse_expect(state, tok, TT_SEMI) < 0) {
        return -1;
    }

    if (ast_alloc_node(state, AST_CONTINUE, &root) < 0) {
        trace_error(state, "failed to allocate AST_CONTINUE\n");
        return -1;
    }

    return cg_compile_node(state, root);
}

/*
 * Begin parsing tokens from the input source
 *
 * @state: Compiler state
 * @tok: Last token
 */
static int
begin_parse(struct gup_state *state, struct token *tok)
{
    if (state == NULL || tok == NULL) {
        errno = -EINVAL;
        return -1;
    }

    switch (tok->type) {
    case TT_ASM:
        if (parse_asm(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_PROC:
        if (parse_proc(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_RBRACE:
        if (parse_rbrace(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_LOOP:
        if (parse_loop(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_BREAK:
        if (parse_break(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_CONT:
        if (parse_continue(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_IDENT:
        if (parse_ident(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_RETURN:
        if (parse_return(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_STRUCT:
        if (parse_struct(state, tok) < 0) {
            return -1;
        }

        break;
    case TT_PUB:
        break;
    case TT_COMMENT:
        break;
    default:
        if (parse_var(state, tok) == 0) {
            break;
        }

        utok(state, tok->type);
        return -1;
    }

    tail_token = *tok;
    return 0;
}

int
gup_parse(struct gup_state *state)
{
    int error = 0;

    if (state == NULL) {
        errno = -EINVAL;
        return -1;
    }

    while (lexer_scan(state, &last_token) == 0) {
        trace_debug("got token %s\n", toktab[last_token.type]);
        if ((error = begin_parse(state, &last_token)) < 0) {
            break;
        }
    }

    if (scope_top(state) != TT_NONE && error == 0) {
        ueof(state);
        trace_warn("missing RBRACE ('}') ?\n");
        return -1;
    }

    return 0;
}
