#pragma once

#include <stdint.h>

typedef struct string string_t;
struct string {
   const char *ptr;
   size_t      len;
};

typedef enum {
   TK_EOF,
   TK_NUM,
   TK_WORD,
   TK_PLUS,
   TK_MINUS,
   TK_STAR,
   TK_CARET,
   TK_SLASH,
   TK_COLON,
   TK_SEMI,
} tok_kind;

typedef struct tok tok_t;
struct tok {
   tok_kind kind;
   string_t text;
};

typedef struct tok_array tok_array_t;
struct tok_array {
   size_t len, cap;
   tok_t *ptr;
};

void tok_array_init(tok_array_t *toks);
void tok_array_push(tok_array_t *toks, tok_t t);
void tok_array_free(tok_array_t *toks);

typedef struct lex lex_t;
struct lex {
   string_t src;
   size_t   read_pos;
   size_t   pos;
   char     ch;
};

void lex_init(lex_t *lx, char const *str);
void lex_to_tokens(lex_t *lx, tok_array_t *toks);

typedef enum {
   OP_ADD,
   OP_SUB,
   OP_DIV,
   OP_MUL,
   OP_POW,
   OP_DROP,
   OP_SWAP,
   OP_DUP,
   OP_LET
} op_kind;

typedef struct op op_t;
struct op {
   op_kind kind;
   union {
      double value;
      struct {
         string_t text;
         uint8_t len;
      } ident;
   } as;
};

typedef struct op_array op_array_t;
struct op_array {
   size_t len, cap;
   op_t  *ptr;
};

typedef struct stack stack_t;
struct stack {
   size_t  len, cap;
   double *ptr;
};

typedef struct vm vm_t;
struct vm {
   stack_t    stack;
   op_array_t ops;
};