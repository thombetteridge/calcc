#include <assert.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "calcc.h"

static inline void memzero(void *ptr, size_t n)
{
   memset(ptr, 0, n);
}

void tok_array_init(tok_array_t *toks)
{
   toks->len = 0;
   toks->cap = 8;
   toks->ptr = (tok_t *)malloc(toks->cap * sizeof(tok_t));
}
void tok_array_push(tok_array_t *toks, tok_t t)
{
   if (toks->len >= toks->cap) {
      toks->cap *= 2;
      toks->ptr = (tok_t *)realloc(toks->ptr, toks->cap * sizeof(tok_t));
   }
   toks->ptr[toks->len] = t;
   ++toks->len;
}
void tok_array_free(tok_array_t *toks)
{
   toks->cap = 0;
   toks->len = 0;
   free(toks->ptr);
   toks->ptr = 0;
}

tok_t *tok_get(tok_array_t const *toks, size_t index)
{
   assert(index < toks->len);
   return &toks->ptr[index];
}

static void lx_advance(lex_t *lx)
{
   if (lx->read_pos >= lx->src.len) {
      lx->ch = '\0';
      return;
   }
   lx->pos = lx->read_pos;
   lx->ch  = lx->src.ptr[lx->pos];
   ++lx->read_pos;
}

void lex_init(lex_t *lx, const char *str)
{
   lx->src.ptr  = str;
   lx->src.len  = strlen(str);
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

static tok_t lx_new_token(lex_t *lx, tok_kind kind)
{
   tok_t result = {
       .kind = kind,
       .text = {.len = 1, .ptr = lx->src.ptr + lx->pos}};
   lx_advance(lx);
   return result;
}

static tok_t lx_new_number(lex_t *lx)
{
   size_t const start = lx->pos;

   while (is_digit(lx->ch) || lx->ch == '.') {
      lx_advance(lx);
   }

   return (tok_t){
       .kind = TK_NUM,
       .text = {.len = lx->pos - start, .ptr = lx->src.ptr + start}};
}

static tok_t lx_new_word(lex_t *lx)
{
   size_t const start = lx->pos;

   while (is_letter(lx->ch) || is_digit(lx->ch)) {
      lx_advance(lx);
   }

   return (tok_t){
       .kind = TK_WORD,
       .text = {.len = lx->pos - start, .ptr = lx->src.ptr + start}};
}

static tok_t lx_next(lex_t *lx)
{
   while (is_white(lx->ch)) {
      lx_advance(lx);
   }

   switch (lx->ch) {
   case '\0': return lx_new_token(lx, TK_EOF);
   case '+': return lx_new_token(lx, TK_PLUS);
   case '-': return lx_new_token(lx, TK_MINUS);
   case '/': return lx_new_token(lx, TK_SLASH);
   case ':': return lx_new_token(lx, TK_COLON);
   case '^': return lx_new_token(lx, TK_CARET);
   case ';': return lx_new_token(lx, TK_SEMI);

   default:
      if (is_digit(lx->ch)) {
         return lx_new_number(lx);
      }
      else {
         return lx_new_word(lx);
      }
   }
}

void lex_to_tokens(lex_t *lx, tok_array_t *toks)
{
   tok_t tok = {};
   do {
      tok = lx_next(lx);
      tok_array_push(toks, tok);
   } while (tok.kind != TK_EOF);
}

void stack_init(stack_t *s)
{
   s->len = 0;
   s->cap = 8;
   s->ptr = (double *)malloc(s->cap * sizeof(double));
}

void stack_push(stack_t *s, double x)
{
   if (s->len >= s->cap) {
      s->cap *= 2;
      s->ptr = (double *)realloc(s->ptr, s->cap * 2);
   }
   s->ptr[s->len] = x;
   ++s->len;
}

double stack_top(stack_t *s)
{
   assert(s->len > 0);
   return s->ptr[s->len - 1];
}

double stack_pop(stack_t *s)
{
   assert(s->len > 0 && "stack underflow");
   double const x = s->ptr[s->len - 1];
   --s->len;
   return x;
}
void stack_free(stack_t *s)
{
   free(s->ptr);
   memzero(s, sizeof(stack_t));
}

void compile(vm_t *vm, tok_array_t *toks)
{
   for (size_t i = 0; /* */; ++i) {
      tok_t const *tok = tok_get(toks, i);
      op_t         op  = {};
      switch (tok->kind) {

      case TK_EOF: return;
      case TK_NUM:
      case TK_WORD:
      case TK_PLUS: op.kind = OP_ADD;
      case TK_MINUS:
      case TK_STAR:
      case TK_CARET:
      case TK_SLASH:
      case TK_COLON:
      case TK_SEMI: break;
      }
   }
}

void eval(vm_t *vm)
{
}