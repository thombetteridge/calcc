#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "base.h"
#include "calcc.h"

static U8 temp_alloc_buffer[4096];


enum StackError {
    STACK_SUCCESS,
    STACK_UNDERFLOW,
    STACK_OVERFLOW,
    STACK_OUT_OF_RANGE,
};

static char const * stack_error_to_stringz(StackError err) {
    switch (err) {

    case STACK_SUCCESS: return "Success";
    case STACK_UNDERFLOW: return "Underflow";
    case STACK_OVERFLOW: return "Overflow";
    case STACK_OUT_OF_RANGE: return "Out of Range";
    default:
        assert(0);
    }
    return 0;
}

inline static void
memzero(void * ptr, size_t n) {
    memset(ptr, 0, n);
}

static void lx_advance(Lexer * lx) {
    if (lx->read_pos >= lx->src.len) {
        lx->pos = lx->read_pos;
        lx->ch  = '\0';
        return;
    }
    lx->pos = lx->read_pos;
    lx->ch  = lx->src.ptr[lx->pos];
    ++lx->read_pos;
}

Lexer lx_init(char const * str, Sz len) {
    Lexer lx = { 0 };

    lx.src.ptr  = str;
    lx.src.len  = len;
    lx.read_pos = 0;
    lx.pos      = 0;
    lx_advance(&lx);

    return lx;
}

static bool is_white(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static Token lx_new_token(Lexer * lx, TokKind kind) {
    Token result = {
        .kind = kind,
        .text = { .len = 1, .ptr = lx->src.ptr + lx->pos }
    };
    lx_advance(lx);
    return result;
}

static char lx_peek(Lexer * lx) {
    if (lx->read_pos >= lx->src.len)
        return '\0';
    else
        return lx->src.ptr[lx->read_pos];
}

static Token lx_new_number(Lexer * lx) {
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

static Token lx_new_word(Lexer * lx) {
    size_t const start = lx->pos;

    while (is_letter(lx->ch) || is_digit(lx->ch)) {
        lx_advance(lx);
    }

    return (Token) {
        .kind = TK_WORD,
        .text = { .len = lx->pos - start, .ptr = lx->src.ptr + start }
    };
}

static Token lx_next(Lexer * lx) {
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
    case '-':
        if (is_digit(lx_peek(lx)))
            return lx_new_number(lx);
        else
            return lx_new_token(lx, TK_MINUS);
    case '/': return lx_new_token(lx, TK_SLASH);
    case '*': return lx_new_token(lx, TK_STAR);
    case ':': return lx_new_token(lx, TK_COLON);
    case '^': return lx_new_token(lx, TK_CARET);
    case ';': return lx_new_token(lx, TK_SEMI);

    default:
        if (is_digit(lx->ch)) {
            return lx_new_number(lx);
        } else if (is_letter(lx->ch)) {
            return lx_new_word(lx);
        } else {
            return lx_new_token(lx, TK_ILLEGAL);
        }
    }
}

void lx_to_tokens(Lexer * lx, TokenArray * toks) {
    Token tok = { 0 };
    do {
        tok = lx_next(lx);
        arr_push(toks, tok);
    } while (tok.kind != TK_EOF);
}


typedef Opt(F64) OptF64;
typedef Opt(struct { F64 x, y; }) OptF64Pair;


static void stack_push(Stack * s, F64 x) {
    arr_push(s, x);
}


static OptF64 stack_top(Stack * s) {
    if (s->len == 0) {
        return OptNone(OptF64);
    }
    return OptSome(OptF64, s->ptr[s->len - 1]);
}

static OptF64 stack_pop(Stack * s) {
    if (s->len == 0) {
        fprintf(stderr, "Stack underflow\n");
        return OptNone(OptF64);
    }
    F64 const x = s->ptr[s->len - 1];
    s->len -= 1;
    return OptSome(OptF64, x);
}

static OptF64Pair stack_pop2(Stack * s) {
    if (s->len < 2)
        return OptNone(OptF64Pair);
    F64 const x = s->ptr[s->len - 1];
    s->len -= 1;
    F64 const y = s->ptr[s->len - 1];
    s->len -= 1;
    return OptSome(OptF64Pair, { .x = x, .y = y });
}

static F64 string_to_F64(StringV s) {
    static char buffer[128];
    Sz const    len = $min(s.len, $cast(Sz, sizeof(buffer) - 1));

    sprintf(buffer, "%.*s", (int)len, s.ptr);
    return atof(buffer);
}


// KEYWORDS

static Sz sv_hash37(StringV s) {
    Sz hash = 0;
    for ($it(i, s.len)) {
        hash = hash * 37 + (Sz)s.ptr[i];
    }
    return hash;
}

static StackError calc_dup(Stack * s) {
    if (s->len == 0) {
        stack_push(s, 0);
        return STACK_UNDERFLOW;
    }
    F64 const top = s->ptr[s->len - 1];
    stack_push(s, top);
    return STACK_SUCCESS;
}

static StackError calc_swap(Stack * s) {
    OptF64Pair const opt = stack_pop2(s);

    if (opt.ok) {
        stack_push(s, opt.value.x);
        stack_push(s, opt.value.y);
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_drop(Stack * s) {
    OptF64 const opt = stack_pop(s);

    if (opt.ok) {
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_clear(Stack * s) {
    arr_clear(s);
    return STACK_SUCCESS;
}

static StackError calc_count(Stack * s) {
    stack_push(s, (F64)s->len);
    return STACK_SUCCESS;
}

static StackError calc_over(Stack * s) {
    if (s->len < 2) {
        stack_push(s, 0);
        return STACK_OUT_OF_RANGE;
    } else {
        stack_push(s, s->ptr[s->len - 2]);
        return STACK_SUCCESS;
    }
}

static StackError calc_roll(Stack * s) {
    if (s->len < 2)
        return STACK_OUT_OF_RANGE;

    F64 const t = stack_top(s).value;

    memmove(s->ptr + 1, s->ptr, sizeof(F64) * (s->len - 1));
    s->ptr[0] = t;
    return STACK_SUCCESS;
}


static StackError calc_sqrt(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {
        stack_push(s, sqrt(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_sin(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {

        stack_push(s, sin(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_cos(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {
        stack_push(s, cos(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_tan(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {
        stack_push(s, tan(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_asin(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {
        stack_push(s, asin(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_acos(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {
        stack_push(s, acos(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_atan(Stack * s) {
    OptF64 const opt = stack_pop(s);
    if (opt.ok) {
        stack_push(s, atan(opt.value));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_atan2(Stack * s) {
    OptF64Pair opt = stack_pop2(s);
    if (opt.ok) {
        F64 const x = opt.value.x;
        F64 const y = opt.value.y;
        stack_push(s, atan2(y, x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_pi(Stack * s) {
    stack_push(s, 3.14159265358979323846);
    return STACK_SUCCESS;
}

static StackError calc_mod(Stack * s) {
    OptF64Pair opt = stack_pop2(s);
    if (opt.ok) {
        F64 const x = opt.value.x;
        F64 const y = opt.value.y;
        stack_push(s, (I32)y % (I32)x);
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_neg(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, x * -1);
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_abs(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, fabs(x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_floor(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, floor(x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_ceil(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, ceil(x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_round(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, round(x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}


static StackError calc_log(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, log(x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static StackError calc_exp(Stack * s) {
    OptF64 opt = stack_pop(s);
    if (opt.ok) {
        F64 const x = opt.value;
        stack_push(s, exp(x));
        return STACK_SUCCESS;
    } else {
        return STACK_UNDERFLOW;
    }
}

static U64 builtin_table_hash(StringV s) {
    U64 hash = 14695981039346656037ULL;

    for ($it(i, s.len)) {
        U8 const c = $cast(U8, s.ptr[i]);
        hash ^= $cast(U64, c);
        hash *= 1099511628211ULL;
    }
    return hash;
}


static void builtin_table_insert(BuiltinTable * t, StringV key, Builtin value) {
    U64 h = builtin_table_hash(key);
    Sz  i = $cast(Sz, h % t->capacity);

    while (t->entries[i].occupied) {
        i = $cast(Sz, ($cast(U64, i + 1) % $cast(U64, t->capacity)));
    }

    BuiltinTableEntry new_entry = {
        .key      = h,
        .value    = value,
        .occupied = true,
    };

    t->entries[i] = new_entry;
}


static bool builtin_table_get(BuiltinTable * t, StringV key, Builtin * value) {
    U64 h = builtin_table_hash(key);
    Sz  i = $cast(Sz, h % t->capacity);


    while (t->entries[i].occupied) {
        if (t->entries[i].key == h) {
            *value = t->entries[i].value;
            return true;
        }
        i = $cast(Sz, ($cast(U64, i + 1) % $cast(U64, t->capacity)));
    }

    return false;
}

#define BUILTIN_TABLE_ENTRIES 64

static BuiltinTable builtins_table_init(void) {
    static bool once = true;

    assert(once && "builtins_table_init called twice");

    once = false;

    BuiltinTable result = { 0 };
    result.capacity     = BUILTIN_TABLE_ENTRIES;
    static BuiltinTableEntry entries[BUILTIN_TABLE_ENTRIES];
    result.entries = entries;

    // stack
    builtin_table_insert(&result, SVLIT("dup"), calc_dup);
    builtin_table_insert(&result, SVLIT("swap"), calc_swap);
    builtin_table_insert(&result, SVLIT("drop"), calc_drop);
    builtin_table_insert(&result, SVLIT("over"), calc_over);
    builtin_table_insert(&result, SVLIT("count"), calc_count);
    builtin_table_insert(&result, SVLIT("roll"), calc_roll);
    builtin_table_insert(&result, SVLIT("clear"), calc_clear);

    // maths
    builtin_table_insert(&result, SVLIT("sqrt"), calc_sqrt);
    builtin_table_insert(&result, SVLIT("sin"), calc_sin);
    builtin_table_insert(&result, SVLIT("cos"), calc_cos);
    builtin_table_insert(&result, SVLIT("tan"), calc_tan);
    builtin_table_insert(&result, SVLIT("asin"), calc_asin);
    builtin_table_insert(&result, SVLIT("acos"), calc_acos);
    builtin_table_insert(&result, SVLIT("atan"), calc_atan);
    builtin_table_insert(&result, SVLIT("atan2"), calc_atan2);
    builtin_table_insert(&result, SVLIT("log"), calc_log);
    builtin_table_insert(&result, SVLIT("exp"), calc_exp);

    builtin_table_insert(&result, SVLIT("mod"), calc_mod);
    builtin_table_insert(&result, SVLIT("neg"), calc_neg);
    builtin_table_insert(&result, SVLIT("abs"), calc_abs);
    builtin_table_insert(&result, SVLIT("floor"), calc_floor);
    builtin_table_insert(&result, SVLIT("ceil"), calc_ceil);
    builtin_table_insert(&result, SVLIT("round"), calc_round);

    // constants
    builtin_table_insert(&result, SVLIT("pi"), calc_pi);

    // for ($it(i, result.capacity)) {
    //     if (result.entries[i].occupied) {
    //         fprintf(stderr, "i:%" PRId64 " k: %" PRIu64 "\n", i, result.entries[i].key);
    //     }
    // }

    return result;
}

static void * arena_allocator_alloc(Allocator * self, size_t size, size_t alignment) {
    (void)(alignment);
    Arena * arena = $ptrCast(Arena, self->ctx);

    return arena_alloc(arena, size);
}

static void arena_allocator_dealloc(Allocator * self, void * ptr, size_t size) {
    (void)self;
    (void)ptr;
    (void)size;
}


static Allocator arena_allocator_init(Arena * arena) {

    Allocator a = {
        .ctx     = arena,
        .alloc   = arena_allocator_alloc,
        .dealloc = arena_allocator_dealloc,
    };
    return a;
}


static void userword_table_init(UserwordTable * user, Sz initial_size) {
    *user = (UserwordTable) { 0 };

    Sz const bytes = sizeof(UserwordTableEntry) * initial_size;
    arena_reserve(&user->arena, bytes * 2 + sizeof(ArenaRegion));

    user->allocator = arena_allocator_init(&user->arena);
    user->entries   = ALLOC(&user->allocator, UserwordTableEntry, initial_size);
    user->capacity  = initial_size;
}

static void userword_table_deinit(UserwordTable * user) {
    arena_destroy(&user->arena);
}

static StringV sv_dup(Allocator * a, StringV s) {
    char * buffer = ALLOC(a, char, s.len);
    memcpy(buffer, s.ptr, s.len);
    return (StringV) { .ptr = buffer, .len = s.len };
}

static TokenArray user_tokens_dup(Allocator * a, TokenArray arr) {
    TokenArray dup_arr = { 0 };

    arr_init(&dup_arr, a);
    arr_reserve(&dup_arr, arr.len);

    for ($it(i, arr.len)) {
        Token dup_t = { .kind = arr.ptr[i].kind, .text = sv_dup(a, arr.ptr[i].text) };
        arr_push(&dup_arr, dup_t);
    }

    return dup_arr;
}

static U64 userword_hash(TokenArray tokens) {
    U64 result = 123456789;

    for ($it(i, tokens.len)) {
        result = result * 33 + sv_hash37(tokens.ptr[i].text);
    }

    return result;
}

static bool sv_key_eq(StringV a, StringV b) {
    if (a.len != b.len)
        return false;
    for ($it(i, a.len)) {
        if (a.ptr[i] != b.ptr[i])
            return false;
    }
    return true;
}

static void userword_table_add(UserwordTable * user, StringV key, TokenArray tokens);

static void userword_table_grow(UserwordTable * user) {
    UserwordTable bigger;
    userword_table_init(&bigger, user->capacity * 2);

    for ($it(i, user->capacity)) {
        if (user->entries[i].occupied)
            userword_table_add(&bigger, user->entries[i].key, user->entries[i].value);
    }

    userword_table_deinit(user);
    *user           = bigger;
    user->allocator = arena_allocator_init(&user->arena);
}

static void userword_table_add(UserwordTable * user, StringV key, TokenArray tokens) {
    if (user->count * 10 >= user->capacity * 7) // 70%
        userword_table_grow(user);

    Sz h = sv_hash37(key) % user->capacity;

    while (user->entries[h].occupied) {
        if (sv_key_eq(user->entries[h].key, key)) {
            // replace if hash if different
            if (userword_hash(tokens) != user->entries[h].hash) {
                user->entries[h].value = user_tokens_dup(&user->allocator, tokens);
                user->entries[h].hash  = userword_hash(tokens);
                return;
            } else {
                // if hash was the same do nothing
                return;
            }
        }
        h = (h + 1) % user->capacity;
    }

    // new entry;
    UserwordTableEntry entry = { 0 };
    entry.key                = sv_dup(&user->allocator, key);
    entry.value              = user_tokens_dup(&user->allocator, tokens);
    entry.occupied           = true;
    entry.hash               = userword_hash(entry.value);
    user->entries[h]         = entry;

    user->count += 1;
}

static bool userword_table_get(UserwordTable * user, StringV key, TokenArray * out) {
    Sz h = sv_hash37(key) % user->capacity;

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

#define BIN_OP(_op_)                                     \
    do {                                                 \
        OptF64Pair const opt = stack_pop2(&calc->stack); \
        if (opt.ok) {                                    \
            F64 const x = opt.value.x;                   \
            F64 const y = opt.value.y;                   \
            stack_push(&calc->stack, y _op_ x);          \
        }                                                \
    } while (0)


static void calc_eval_tokens(Calculator * calc, TokenArray const * tokens) {
    for ($it(i, tokens->len)) {
        Token tok = tokens->ptr[i];

        switch (tok.kind) {

        case TK_EOF: return;
        case TK_NUM: stack_push(&calc->stack, string_to_F64(tok.text)); break;

        case TK_WORD: {
            Builtin builtin;
            if (builtin_table_get(&calc->builtins, tok.text, &builtin)) {
                builtin(&calc->stack);
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
        case TK_PLUS: BIN_OP(+); break;
        case TK_MINUS: BIN_OP(-); break;
        case TK_STAR: BIN_OP(*); break;
        case TK_SLASH: BIN_OP(/); break;
        case TK_CARET: {
            if (calc->stack.len < 2) {
                fprintf(stderr, "bin_op underflow '^'");
            } else {
                OptF64Pair const opt = stack_pop2(&calc->stack);
                if (opt.ok) {
                    F64 const x = opt.value.x;
                    F64 const y = opt.value.y;
                    stack_push(&calc->stack, pow(y, x));
                }
            }
            break;
        }
        case TK_COLON: {
            StringV    word_name  = { 0 };
            TokenArray definition = { 0 };
            Allocator  temp_alloc = fixed_allocator_init(temp_alloc_buffer, sizeof(temp_alloc_buffer));

            arr_init(&definition, &temp_alloc);
            arr_reserve(&definition, 32);

            if (++i < tokens->len && tokens->ptr[i].kind == TK_WORD) {
                word_name = tokens->ptr[i].text;
            } else {
                break;
            }

            while (++i < tokens->len && tokens->ptr[i].kind != TK_SEMI) {
                arr_push(&definition, tokens->ptr[i]);
            }

            if (i < tokens->len && tokens->ptr[i].kind == TK_SEMI) {
                userword_table_add(&calc->userwords, word_name, definition);
            } else {
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


Calculator calc_init(Allocator * allocator) {
    Calculator calc = { 0 };
    calc.allocator  = allocator;

    arr_init(&calc.stack, calc.allocator);
    arr_init(&calc.tokens, calc.allocator);
    calc.builtins = builtins_table_init();

    Sz const inital_buffer_len = 2048;

    calc.output_buffer = ALLOC(calc.allocator, char, inital_buffer_len);
    calc.output_len    = inital_buffer_len;


    userword_table_init(&calc.userwords, 64);

    return calc;
}

void calc_deinit(Calculator * calc) {
    arr_deinit(&calc->stack);
    arr_deinit(&calc->tokens);
    DEALLOC(calc->allocator, calc->output_buffer, calc->output_len);
    userword_table_deinit(&calc->userwords);
}

StringV calc_eval(Calculator * calc, StringV src) {
    calc->lx = lx_init(src.ptr, src.len);
    arr_clear(&calc->tokens);

    lx_to_tokens(&calc->lx, &calc->tokens);

    arr_clear(&calc->stack);

    calc_eval_tokens(calc, &calc->tokens);

    Sz offset = 0;
    for ($it(i, calc->stack.len)) {
        if (offset + 64 > calc->output_len) {
            Sz     new_len    = calc->output_len * 2;
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
