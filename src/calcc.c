#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
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

void lx_init(lex_t *lx, const char *str)
{
   lx->src.ptr  = str;
   lx->src.len  = strlen(str)+1;
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
   case '\0': {
      tok_t result = {
          .kind = TK_EOF,
          .text = {.len = sizeof("EOF"), .ptr = "EOF"}};
      return result;
   }
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

void lx_to_tokens(lex_t *lx, tok_array_t *toks)
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

void stack_clear(stack_t *s)
{
   s->len = 0;
}

void stack_free(stack_t *s)
{
   free(s->ptr);
   memzero(s, sizeof(stack_t));
}

double string_to_double(string_t s)
{
   static char buffer[128];
   sprintf(buffer, "%*.s", (int)s.len, s.ptr);
   return atof(buffer);
}

#define BIN_OP(_op_)                                                    \
   do {                                                                 \
      if (stack->len < 2) { fprintf(stderr, "bin_op underflow flow"); } \
      double const x = stack_pop(stack);                                \
      double const y = stack_pop(stack);                                \
      stack_push(stack, y _op_ x);                                      \
   } while (0)

void eval_tokens(tok_array_t *toks, stack_t *stack)
{
   for (size_t i = 0; i < toks->len; ++i) {
      tok_t tok = *tok_get(toks, i);

      switch (tok.kind) {

      case TK_EOF:
         return;
      case TK_NUM:
         stack_push(stack, string_to_double(tok.text));
         break;
      case TK_WORD:
         assert(0 && "WORDS TODO");
      case TK_PLUS:
         BIN_OP(+);
      case TK_MINUS:
         BIN_OP(-);
      case TK_STAR:
         BIN_OP(*);
      case TK_CARET: {
         if (stack->len < 2) {
            fprintf(stderr, "bin_op underflow flow");
            double const x = stack_pop(stack);
            double const y = stack_pop(stack);
            stack_push(stack, pow(y, x));
         }
      }
      case TK_SLASH:
         BIN_OP(/);
      case TK_COLON:
      case TK_SEMI: break;
      }
   }
}

void eval(tok_array_t *toks, char *output)
{
   static stack_t stack;
   static int     once = 1;
   if (once) {
      once = 0;
      stack_init(&stack);
   }
   stack_clear(&stack);

   eval_tokens(toks, &stack);

   size_t offset = 0;
   for (size_t i = 0; i < stack.len; ++i) {
      offset += sprintf(output + offset, "%g\n", stack.ptr[i]);
   }
}