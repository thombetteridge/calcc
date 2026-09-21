#pragma once

#include <stdint.h>

#include "arena.h"
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
    Sz      len, cap;

    Allocator * allocator;
};


typedef struct Lexer Lexer;
struct Lexer {
    StringV src;
    Sz      read_pos;
    Sz      pos;
    char    ch;
};

Lexer lx_init(char const * str, Sz len);
void  lx_to_tokens(Lexer * lx, TokenArray * toks);

typedef struct Stack Stack;
struct Stack {
    Sz       len, cap;
    double * ptr;

    Allocator * allocator;
};

typedef int StackError;

typedef StackError (*Builtin)(Stack *);

typedef struct BuiltinTableEntry BuiltinTableEntry;
struct BuiltinTableEntry {
    U64     key;
    Builtin value;
    bool    occupied;
};

typedef struct BuiltinTable BuiltinTable;
struct BuiltinTable {
    BuiltinTableEntry * entries;
    Sz                  count, capacity;
};


typedef struct UserwordTableEntry UserwordTableEntry;
struct UserwordTableEntry {
    StringV    key;
    TokenArray value;
    U64        hash;
    bool       occupied;
};

typedef struct UserwordTable UserwordTable;
struct UserwordTable {
    UserwordTableEntry * entries;
    Sz                   count, capacity;


    Allocator allocator;
    Arena     arena;
};

typedef struct Calculator Calculator;
struct Calculator {
    Lexer         lx;
    TokenArray    tokens;
    Stack         stack;
    BuiltinTable  builtins;
    UserwordTable userwords;

    char * output_buffer;
    Sz     output_len;

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
//    Sz len, cap;
//    op_t  *ptr;
// };
//

// typedef struct vm vm_t;
// struct vm {
//    Stack    stack;
//    op_array_t ops;
// };
