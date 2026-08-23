#pragma once

#include <stdint.h>

#include "base.h"


typedef enum {
    TK_EOF,
    TK_ILLEGAL,
    TK_NUM,
    TK_WORD,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_CARET,
    TK_SLASH,
    TK_COLON,
    TK_SEMI,
} TokKind;

typedef struct Token Token;
struct Token {
    TokKind kind;
    StringV text;
};

typedef struct TokenArray TokenArray;
struct TokenArray {
    Token * ptr;
    usize   len, cap;

    Allocator * allocator;
};


typedef struct Lexer Lexer;
struct Lexer {
    StringV src;
    usize   read_pos;
    usize   pos;
    char    ch;
};

void lx_init(Lexer * lx, char const * str, usize len);
void lx_to_tokens(Lexer * lx, TokenArray * toks);

typedef struct Stack Stack;
struct Stack {
    usize    len, cap;
    double * ptr;

    Allocator * allocator;
};


typedef struct Keyword_Table_Entry Keyword_Table_Entry;
struct Keyword_Table_Entry {
    StringV key;
    void (*value)(Stack *);
    bool occupied;
};

typedef struct Keyword_Table Keyword_Table;
struct Keyword_Table {
    Keyword_Table_Entry * entries;
    usize                 count, capacity;

    Allocator * allocator;
};

typedef struct Calculator Calculator;
struct Calculator {
    Stack         stack;
    Keyword_Table keywords;

    Allocator allocator;
};

Calculator calc_init(Allocator * allocator);
void calc_deinit(Calculator* calc);

usize calc_eval(Calculator* calc, TokenArray * toks, char * output);


// typedef enum {
//    OP_ADD,
//    OP_SUB,
//    OP_DIV,
//    OP_MUL,
//    OP_POW,
//    OP_DROP,
//    OP_SWAP,
//    OP_DUP,
//    OP_LET
// } op_kind;

// typedef struct op op_t;
// struct op {
//    op_kind kind;
//    union {
//       double value;
//       struct {
//          StringV text;
//          uint8_t len;
//       } ident;
//    } as;
// };

// typedef struct op_array op_array_t;
// struct op_array {
//    usize len, cap;
//    op_t  *ptr;
// };
//

// typedef struct vm vm_t;
// struct vm {
//    Stack    stack;
//    op_array_t ops;
// };
