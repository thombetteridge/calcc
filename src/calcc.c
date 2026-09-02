#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "calcc.h"

static u8 temp_alloc_buffer[4096];


inline static void
memzero(void * ptr, size_t n)
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

Lexer lx_init(char const * str, usize len)
{
    Lexer lx = { 0 };

    lx.src.ptr  = str;
    lx.src.len  = len;
    lx.read_pos = 0;
    lx.pos      = 0;
    lx_advance(&lx);

    return lx;
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

static char lx_peek(Lexer * lx)
{
    if (lx->read_pos >= lx->src.len)
        return '\0';
    else
        return lx->src.ptr[lx->read_pos];
}

static Token lx_new_number(Lexer * lx)
{
    size_t const start = lx->pos;

    if (lx->ch == '-')
        lx_advance(lx);

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
        if (is_digit(lx_peek(lx)))
            return lx_new_number(lx);
        else
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


static void stack_push(Stack * s, f64 x)
{
    arr_push(s, x);
}


static f64 stack_top(Stack * s)
{
    if (s->len == 0) {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    return s->ptr[s->len - 1];
}

static f64 stack_pop(Stack * s)
{
    if (s->len == 0) {
        fprintf(stderr, "Stack underflow\n");
        return 0;
    }
    f64 const x = s->ptr[s->len - 1];
    s->len -= 1;
    return x;
}

static f64 string_to_f64(StringV s)
{
    static char buffer[128];
    usize const len = Min(s.len, sizeof(buffer) - 1);

    sprintf(buffer, "%.*s", (int)len, s.ptr);
    return atof(buffer);
}


// KEYWORDS

static usize sv_hash37(StringV s)
{
    usize hash = 0;
    for (iterate(i, s.len)) {
        hash = hash * 37 + (usize)s.ptr[i];
    }
    return hash;
}

static void calc_dup(Stack * s)
{
    if (s->len == 0) {
        stack_push(s, 0);
        return;
    }
    f64 const top = s->ptr[s->len - 1];
    stack_push(s, top);
}

static void calc_swap(Stack * s)
{
    f64 const x = stack_pop(s);
    f64 const y = stack_pop(s);

    stack_push(s, x);
    stack_push(s, y);
}

static void calc_drop(Stack * s)
{
    f64 const x = stack_pop(s);
    (void)x;
}

static void calc_clear(Stack * s)
{
    arr_clear(s);
}

static void calc_count(Stack * s)
{
    stack_push(s, (f64)s->len);
}

static void calc_over(Stack * s)
{
    if (s->len < 2)
        stack_push(s, 0);
    else
        stack_push(s, s->ptr[s->len - 2]);
}

static void calc_roll(Stack * s)
{
    if (s->len < 2)
        return;
    f64 const t = stack_top(s);

    memmove(s->ptr + 1, s->ptr, sizeof(f64) * (s->len - 1));
    s->ptr[0] = t;
}


static void calc_sqrt(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, sqrt(x));
}

static void calc_sin(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, sin(x));
}

static void calc_cos(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, cos(x));
}

static void calc_tan(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, tan(x));
}

static void calc_asin(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, asin(x));
}

static void calc_acos(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, acos(x));
}

static void calc_atan(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, atan(x));
}

static void calc_atan2(Stack * s)
{
    f64 const x = stack_pop(s);
    f64 const y = stack_pop(s);
    stack_push(s, atan2(y, x));
}

static void calc_pi(Stack * s)
{
    stack_push(s, 3.14159265358979323846);
}

static void calc_mod(Stack * s)
{
    f64 const x = stack_pop(s);
    f64 const y = stack_pop(s);
    stack_push(s, (i32)y % (i32)x);
}

static void calc_neg(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, x * -1);
}

static void calc_abs(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, fabs(x));
}

static void calc_floor(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, floor(x));
}

static void calc_ceil(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, ceil(x));
}

static void calc_round(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, round(x));
}


static void calc_log(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, log(x));
}

static void calc_exp(Stack * s)
{
    f64 const x = stack_pop(s);
    stack_push(s, exp(x));
}


static void keyword_table_insert(KeywordTable * t, StringV key, void (*value)(Stack *))
{
    usize h = sv_hash37(key) % t->capacity;

    while (t->entries[h].occupied) {
        h = (h + 1) % t->capacity;
    }

    KeywordTableEntry new_entry = {
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

static bool keyword_table_get(KeywordTable * t, StringV key, void (**value)(Stack *))
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

static KeywordTable keywords_table_init(Allocator * a)
{
    KeywordTable result = { 0 };
    result.capacity     = 256;
    result.allocator    = a;
    result.entries      = ALLOC(result.allocator, KeywordTableEntry, result.capacity);

    // stack
    keyword_table_insert(&result, SVLIT("dup"), calc_dup);
    keyword_table_insert(&result, SVLIT("swap"), calc_swap);
    keyword_table_insert(&result, SVLIT("drop"), calc_drop);
    keyword_table_insert(&result, SVLIT("over"), calc_over);
    keyword_table_insert(&result, SVLIT("count"), calc_count);
    keyword_table_insert(&result, SVLIT("roll"), calc_roll);
    keyword_table_insert(&result, SVLIT("clear"), calc_clear);

    // maths
    keyword_table_insert(&result, SVLIT("sqrt"), calc_sqrt);
    keyword_table_insert(&result, SVLIT("sin"), calc_sin);
    keyword_table_insert(&result, SVLIT("cos"), calc_cos);
    keyword_table_insert(&result, SVLIT("tan"), calc_tan);
    keyword_table_insert(&result, SVLIT("asin"), calc_asin);
    keyword_table_insert(&result, SVLIT("acos"), calc_acos);
    keyword_table_insert(&result, SVLIT("atan"), calc_atan);
    keyword_table_insert(&result, SVLIT("atan2"), calc_atan2);
    keyword_table_insert(&result, SVLIT("log"), calc_log);
    keyword_table_insert(&result, SVLIT("exp"), calc_exp);

    keyword_table_insert(&result, SVLIT("mod"), calc_mod);
    keyword_table_insert(&result, SVLIT("neg"), calc_neg);
    keyword_table_insert(&result, SVLIT("abs"), calc_abs);
    keyword_table_insert(&result, SVLIT("floor"), calc_floor);
    keyword_table_insert(&result, SVLIT("ceil"), calc_ceil);
    keyword_table_insert(&result, SVLIT("round"), calc_round);

    // constants
    keyword_table_insert(&result, SVLIT("pi"), calc_pi);

    for (iterate(i, result.capacity)) {
        if (result.entries[i].occupied) {
            fprintf(stderr, "i:%zu k:'" SVFMT "'\n", i, SVARGS(result.entries[i].key));
        }
    }

    return result;
}

static void keywords_table_deinit(KeywordTable * t)
{
    DEALLOC(t->allocator, t->entries, t->capacity);
}

static UserwordTable userword_table_init(Allocator * a, usize inital_size)
{
    UserwordTable user = { 0 };

    user.allocator = a;

    usize const inital_buffer_size = sizeof(UserwordTableEntry) * inital_size + // inital array cap,
                                     24 * inital_size +                         // assume keys of max len 24
                                     sizeof(Token) * 10 * inital_size;          // assume 10 tokens per word?

    u8 * buffer = ALLOC(user.allocator, u8, inital_buffer_size);
    user.buffer = fixed_allocator_init(buffer, inital_buffer_size);

    user.entries  = ALLOC(&user.buffer, UserwordTableEntry, inital_size);
    user.capacity = inital_size;

    return user;
}

static void userword_table_deinit(UserwordTable * user)
{
    FixedAllocator * fixed = (FixedAllocator *)(user->buffer.ctx);
    DEALLOC(user->allocator, fixed, fixed->capacity);
}

StringV sv_dup(Allocator * a, StringV s)
{
    char * buffer = ALLOC(a, char, s.len);
    memcpy(buffer, s.ptr, s.len);
    return (StringV) { .ptr = buffer, .len = s.len };
}

static TokenArray user_tokens_dup(Allocator * a, TokenArray arr)
{
    TokenArray dup_arr = { 0 };
    dup_arr.ptr        = ALLOC(a, Token, arr.len);
    dup_arr.cap        = arr.len;

    for (iterate(i, arr.len)) {
        Token dup_t    = { .kind = arr.ptr[i].kind, .text = sv_dup(a, arr.ptr[i].text) };
        dup_arr.ptr[i] = dup_t;
        dup_arr.len += 1;
    }

    return dup_arr;
}


static void userword_table_add(UserwordTable * user, StringV key, TokenArray tokens)
{
    usize h = sv_hash37(key) % user->capacity;

    // TODO CHECK FOR RUNNING OUT OF SPACE AND REALLOC

    while (user->entries[h].occupied) {
        if (sv_key_eq(user->entries[h].key, key)) {
            // replace
            user->entries[h].value = user_tokens_dup(&user->buffer, tokens);
            return;
        }
        h = (h + 1) % user->capacity;
    }

    // new entry;
    UserwordTableEntry entry = { 0 };
    entry.key                = sv_dup(&user->buffer, key);
    entry.value              = user_tokens_dup(&user->buffer, tokens);
    entry.occupied           = true;
    user->entries[h]         = entry;

    user->count += 1;
}

static bool userword_table_get(UserwordTable * user, StringV key, TokenArray * out)
{
    usize h = sv_hash37(key) % user->capacity;

    while (user->entries[h].occupied) {
        if (sv_key_eq(user->entries[h].key, key)) {
            *out = user->entries[h].value;
            return true;
        }
        h = (h + 1) % user->capacity;
    }
    return false;
}

//

#define BIN_OP(_op_)                                           \
    do {                                                       \
        if (calc->stack.len < 2) {                             \
            fprintf(stderr, "bin_op underflow '" #_op_ "'\n"); \
        }                                                      \
        f64 const x = stack_pop(&calc->stack);                 \
        f64 const y = stack_pop(&calc->stack);                 \
        stack_push(&calc->stack, y _op_ x);                      \
    } while (0)


static void calc_eval_tokens(Calculator * calc, TokenArray const * tokens)
{
    for (iterate(i, tokens->len)) {
        Token tok = tokens->ptr[i];

        switch (tok.kind) {

        case TK_EOF:
            return;
        case TK_NUM:
            stack_push(&calc->stack, string_to_f64(tok.text));
            break;
        case TK_WORD: {
            void (*keyword)(Stack *);
            if (keyword_table_get(&calc->keywords, tok.text, &keyword)) {
                keyword(&calc->stack);
                break;
            }

            TokenArray user_word = { 0 };
            if ((userword_table_get(&calc->userwords, tok.text, &user_word))) {
                calc_eval_tokens(calc, &user_word);
                break;
            }

            fprintf(stderr, "Unknown Word:'" SVFMT "'\n", SVARGS(tok.text));

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
            if (calc->stack.len < 2) {
                fprintf(stderr, "bin_op underflow '^'");
            }
            else {
                f64 const x = stack_pop(&calc->stack);
                f64 const y = stack_pop(&calc->stack);
                stack_push(&calc->stack, pow(y, x));
            }
            break;
        }
        case TK_SLASH:
            BIN_OP(/);
            break;
        case TK_COLON: {
            StringV    word_name  = { 0 };
            TokenArray definition = { 0 };
            Allocator  temp_alloc = fixed_allocator_init(temp_alloc_buffer, sizeof(temp_alloc_buffer));

            arr_init(&definition, &temp_alloc);
            arr_reserve(&definition, 32);

            if (++i < tokens->len && tokens->ptr[i].kind == TK_WORD) {
                word_name = tokens->ptr[i].text;
            }
            else {
                break;
            }

            while (++i < tokens->len && tokens->ptr[i].kind != TK_SEMI) {
                arr_push(&definition, tokens->ptr[i]);
            }

            if (i < tokens->len && tokens->ptr[i].kind == TK_SEMI) {
                userword_table_add(&calc->userwords, word_name, definition);
            }
            else {
                fprintf(stderr, "unterminated ':'\n");
            }
        } break;
        case TK_SEMI:
            break;
        case TK_ILLEGAL:
            break;
        }
    }
}


Calculator calc_init(Allocator * allocator)
{
    Calculator calc = { 0 };
    calc.allocator  = allocator;

    arr_init(&calc.stack, calc.allocator);
    arr_init(&calc.tokens, calc.allocator);
    calc.keywords = keywords_table_init(calc.allocator);

    usize const inital_buffer_len = 2048;

    calc.output_buffer = ALLOC(calc.allocator, char, inital_buffer_len);
    calc.output_len    = inital_buffer_len;

    calc.userwords = userword_table_init(calc.allocator, 64);

    return calc;
}

void calc_deinit(Calculator * calc)
{
    arr_deinit(&calc->stack);
    arr_deinit(&calc->tokens);
    DEALLOC(calc->allocator, calc->output_buffer, calc->output_len);
    userword_table_deinit(&calc->userwords);
    keywords_table_deinit(&calc->keywords);
}


StringV calc_eval(Calculator * calc, StringV src)
{
    calc->lx = lx_init(src.ptr, src.len);
    arr_clear(&calc->tokens);

    lx_to_tokens(&calc->lx, &calc->tokens);

    arr_clear(&calc->stack);

    calc_eval_tokens(calc, &calc->tokens);

    usize offset = 0;
    for (iterate(i, calc->stack.len)) {
        if (offset + 64 > calc->output_len) {
            usize  new_len    = calc->output_len * 2;
            char * new_buffer = ALLOC(calc->allocator, char, new_len);
            memcpy(new_buffer, calc->output_buffer, calc->output_len);
            DEALLOC(calc->allocator, calc->output_buffer, calc->output_len);

            calc->output_buffer = new_buffer;
            calc->output_len    = new_len;
        }

        offset += sprintf(calc->output_buffer + offset, "%g\n", calc->stack.ptr[i]);
    }

    return (StringV) { .ptr = calc->output_buffer, .len = offset };
}
