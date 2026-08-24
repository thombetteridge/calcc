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

Lexer lx_init(char const * str, usize len);
void  lx_to_tokens(Lexer * lx, TokenArray * toks);

typedef struct Stack Stack;
struct Stack {
    usize    len, cap;
    double * ptr;

    Allocator * allocator;
};


typedef struct KeywordTableEntry KeywordTableEntry;
struct KeywordTableEntry {
    StringV key;
    void (*value)(Stack *);
    bool occupied;
};

typedef struct KeywordTable KeywordTable;
struct KeywordTable {
    KeywordTableEntry * entries;
    usize               count, capacity;

    Allocator * allocator;
};


typedef struct UserwordTableEntry UserwordTableEntry;
struct UserwordTableEntry {
    StringV    key;
    TokenArray value;
    bool       occupied;
};

typedef struct UserwordTable UserwordTable;
struct UserwordTable {
    UserwordTableEntry * entries;
    usize                count, capacity;


    Allocator * allocator;
    Allocator   buffer; //  fixed size buffer;
};

typedef struct Calculator Calculator;
struct Calculator {
    Lexer         lx;
    TokenArray    tokens;
    Stack         stack;
    KeywordTable  keywords;
    UserwordTable userwords;

    char * output_buffer;
    usize  output_len;

    Allocator * allocator;
};

Calculator calc_init(Allocator * allocator);
void       calc_deinit(Calculator * calc);

StringV calc_eval(Calculator * calc, StringV src);


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
