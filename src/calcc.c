#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "calcc.h"







inline static void memzero(void * ptr, size_t n)
{
    memset(ptr, 0, n);
}

static void lx_advance(Lexer * lx)
{
    if (lx->read_pos >= lx->src.len) {
        lx->pos = lx->read_pos;
        lx->ch  = '\0';
        return;
    }
    lx->pos = lx->read_pos;
    lx->ch  = lx->src.ptr[lx->pos];
    ++lx->read_pos;
}

void lx_init(Lexer * lx, char const * str, usize len)
{
    lx->src.ptr  = str;
    lx->src.len  = len;
    lx->read_pos = 0;
    lx->pos      = 0;
    lx_advance(lx);
}

static bool is_white(char c)
{
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static Token lx_new_token(Lexer * lx, TokKind kind)
{
    Token result = {
        .kind = kind,
        .text = { .len = 1, .ptr = lx->src.ptr + lx->pos }
    };
    lx_advance(lx);
    return result;
}

static Token lx_new_number(Lexer * lx)
{
    size_t const start = lx->pos;

    while (is_digit(lx->ch) || lx->ch == '.') {
        lx_advance(lx);
    }

    return (Token) {
        .kind = TK_NUM,
        .text = { .len = lx->pos - start, .ptr = lx->src.ptr + start }
    };
}

static Token lx_new_word(Lexer * lx)
{
    size_t const start = lx->pos;

    while (is_letter(lx->ch) || is_digit(lx->ch)) {
        lx_advance(lx);
    }

    return (Token) {
        .kind = TK_WORD,
        .text = { .len = lx->pos - start, .ptr = lx->src.ptr + start }
    };
}

static Token lx_next(Lexer * lx)
{
    while (is_white(lx->ch)) {
        lx_advance(lx);
    }

    switch (lx->ch) {
    case '\0': {
        return (Token) {
            .kind = TK_EOF,
            .text = { .len = sizeof("EOF"), .ptr = "EOF" }
        };
    }
    case '+':
        return lx_new_token(lx, TK_PLUS);
    case '-':
        return lx_new_token(lx, TK_MINUS);
    case '/':
        return lx_new_token(lx, TK_SLASH);
    case '*':
        return lx_new_token(lx, TK_STAR);
    case ':':
        return lx_new_token(lx, TK_COLON);
    case '^':
        return lx_new_token(lx, TK_CARET);
    case ';':
        return lx_new_token(lx, TK_SEMI);

    default:
        if (is_digit(lx->ch)) {
            return lx_new_number(lx);
        }
        else if (is_letter(lx->ch)) {
            return lx_new_word(lx);
        }
        else {
            return lx_new_token(lx, TK_ILLEGAL);
        }
    }
}

void lx_to_tokens(Lexer * lx, TokenArray * toks)
{
    Token tok = { 0 };
    do {
        tok = lx_next(lx);
        arr_push(toks, tok);
    } while (tok.kind != TK_EOF);
}


double stack_top(Stack * s)
{
    if (s->len == 0) {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    return s->ptr[s->len - 1];
}

double stack_pop(Stack * s)
{
    if (s->len == 0) {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    double const x = s->ptr[s->len - 1];
    s->len -= 1;
    return x;
}

double string_to_double(StringV s)
{
    static char buffer[128];
    usize const len = Min(s.len, sizeof(buffer) - 1);

    sprintf(buffer, "%.*s", (int)len, s.ptr);
    return atof(buffer);
}


// KEYWORDS

usize sv_hash37(StringV s)
{
    usize hash = 123456789;
    for (iterate(i, s.len)) {
        hash = hash * 37 + (usize)s.ptr[i];
    }
    return hash;
}

static void calc_dup(Stack * s)
{
    if (s->len == 0) {
        arr_push(s, 0);
        return;
    }
    double const top = s->ptr[s->len - 1];
    arr_push(s, top);
}

static void calc_swap(Stack * s)
{
    double const x = stack_pop(s);
    double const y = stack_pop(s);

    arr_push(s, x);
    arr_push(s, y);
}

static void calc_sqrt(Stack * s)
{
    double const x = stack_pop(s);
    arr_push(s, sqrt(x));
}

static void calc_sin(Stack * s)
{
    double const x = stack_pop(s);
    arr_push(s, sin(x));
}

static void calc_cos(Stack * s)
{
    double const x = stack_pop(s);
    arr_push(s, cos(x));
}

static void calc_tan(Stack * s)
{
    double const x = stack_pop(s);
    arr_push(s, tan(x));
}


void keyword_table_insert(Keyword_Table * t, StringV key, void (*value)(Stack *))
{
    usize h = sv_hash37(key) % t->capacity;

    while (t->entries[h].occupied) {
        h = (h + 1) % t->capacity;
    }

    Keyword_Table_Entry new_entry = {
        .key      = key,
        .value    = value,
        .occupied = true,
    };

    t->entries[h] = new_entry;
}

static bool sv_key_eq(StringV a, StringV b)
{
    if (a.len != b.len)
        return false;
    for (iterate(i, a.len)) {
        if (a.ptr[i] != b.ptr[i])
            return false;
    }
    return true;
}

bool keyword_table_get(Keyword_Table * t, StringV key, void (**value)(Stack *))
{
    usize h = sv_hash37(key) % t->capacity;

    while (t->entries[h].occupied) {
        if (sv_key_eq(t->entries[h].key, key)) {
            *value = t->entries[h].value;
            return true;
        }
        h = (h + 1) % t->capacity;
    }

    return false;
}

Keyword_Table keywords_table_init(Allocator * a)
{
    Keyword_Table result = { 0 };
    result.capacity      = 256;
    result.allocator     = a;
    result.entries       = ALLOC(result.allocator, Keyword_Table_Entry, result.capacity);

    // stack
    keyword_table_insert(&result, SVLIT("dup"), calc_dup);
    keyword_table_insert(&result, SVLIT("swap"), calc_swap);

    // maths
    keyword_table_insert(&result, SVLIT("sqrt"), calc_sqrt);
    keyword_table_insert(&result, SVLIT("sin"), calc_sin);
    keyword_table_insert(&result, SVLIT("cos"), calc_cos);
    keyword_table_insert(&result, SVLIT("tan"), calc_tan);

    for (iterate(i, result.capacity))
    {
        if (result.entries[i].occupied)
        {
            fprintf(stderr, "i:%zu k:'"SVFMT"'\n",i, SVARGS(result.entries[i].key));
        }
    }

    return result;
}


//

#define BIN_OP(_op_)                                           \
    do {                                                       \
        if (stack->len < 2) {                                  \
            fprintf(stderr, "bin_op underflow '" #_op_ "'\n"); \
        }                                                      \
        double const x = stack_pop(stack);                     \
        double const y = stack_pop(stack);                     \
        arr_push(stack, y _op_ x);                             \
    } while (0)


void eval_tokens(TokenArray * toks, Stack * stack)
{
    for (size_t i = 0; i < toks->len; ++i) {
        Token tok = toks->ptr[i];

        switch (tok.kind) {

        case TK_EOF:
            return;
        case TK_NUM:
            arr_push(stack, string_to_double(tok.text));
            break;
        case TK_WORD: {
            void (*keyword)(Stack *);
            if (keyword_table_get(&KEYWORD_TABLE, tok.text, &keyword)) {
                keyword(stack);
            }
            else {
                fprintf(stderr, "Unknown Word:'" SVFMT "'\n", SVARGS(tok.text));
            }
            break;
        }
        case TK_PLUS:
            BIN_OP(+);
            break;
        case TK_MINUS:
            BIN_OP(-);
            break;
        case TK_STAR:
            BIN_OP(*);
            break;
        case TK_CARET: {
            if (stack->len < 2) {
                fprintf(stderr, "bin_op underflow '^'");
            }
            else {
                double const x = stack_pop(stack);
                double const y = stack_pop(stack);
                arr_push(stack, pow(y, x));
            }
            break;
        }
        case TK_SLASH:
            BIN_OP(/);
            break;
        case TK_COLON:
            break;
        case TK_SEMI:
            break;
        case TK_ILLEGAL:
            break;
        }
    }
}


Calculator calc_init(Allocator *allocator)
{


}

usize eval(Allocator * allocator, TokenArray * toks, char * output)
{
    static Stack stack;
    static int   once = 1;
    if (once) {
        once = 0;
        arr_init(&stack, allocator);
        KEYWORD_TABLE = keywords_table_init(allocator);
    }
    arr_clear(&stack);

    eval_tokens(toks, &stack);

    size_t offset = 0;
    for (size_t i = 0; i < stack.len; ++i) {
        offset += sprintf(output + offset, "%g\n", stack.ptr[i]);
    }

    return offset;
}
