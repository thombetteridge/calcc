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
        lx->ch = '\0';
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
    case '+': return lx_new_token(lx, TK_PLUS);
    case '-': return lx_new_token(lx, TK_MINUS);
    case '/': return lx_new_token(lx, TK_SLASH);
    case '*': return lx_new_token(lx, TK_STAR);
    case ':': return lx_new_token(lx, TK_COLON);
    case '^': return lx_new_token(lx, TK_CARET);
    case ';': return lx_new_token(lx, TK_SEMI);

    default:
        if (is_digit(lx->ch)) {
            return lx_new_number(lx);
        }
        else if (is_letter(lx->ch)){
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
    assert(s->len > 0);
    return s->ptr[s->len - 1];
}

double stack_pop(Stack * s)
{
    assert(s->len > 0 && "stack underflow");
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

#define BIN_OP(_op_)                                  \
    do {                                              \
        if (stack->len < 2) {                         \
            fprintf(stderr, "bin_op underflow flow"); \
        }                                             \
        double const x = stack_pop(stack);            \
        double const y = stack_pop(stack);            \
        arr_push(stack, y  _op_  x);                    \
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
        case TK_WORD:
            assert(0 && "WORDS TODO");
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
                fprintf(stderr, "bin_op underflow flow");
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

usize eval(Allocator * allocator, TokenArray * toks, char * output)
{
    static Stack stack;
    static int   once = 1;
    if (once) {
        once = 0;
        arr_init(&stack, allocator);
    }
    arr_clear(&stack);

    eval_tokens(toks, &stack);

    size_t offset = 0;
    for (size_t i = 0; i < stack.len; ++i) {
        offset += sprintf(output + offset, "%g\n", stack.ptr[i]);
    }

    return offset;
}