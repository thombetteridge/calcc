#include "eval.h"

#include "common.h"
#include "gc/gc.h"
#include "gcstring.h"
#include "lexer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Stack {
   double* data;
   uint    len;
   uint    cap;
} Stack;

static void stack_push(Stack* stack, double x) {
   if (stack->cap == 0) {
      stack->cap  = 8;
      stack->data = (double*)GC_malloc(stack->cap * sizeof(double));
   }

   if (stack->len == stack->cap) {
      stack->cap *= 2;
      stack->data = (double*)GC_realloc(stack->data, stack->cap * sizeof(double));
   }

   stack->data[stack->len] = x;
   stack->len++;
}

static bool stack_top(const Stack* stack, double* out) {
   if (stack->len == 0) return false;
   *out = stack->data[stack->len - 1];
   return true;
}

static bool stack_pop(Stack* stack, double* out) {
   if (stack->len == 0) return false;
   stack_top(stack, out);
   stack->len--;
   return true;
}

static bool binop_pop2(Stack* st, double* a, double* b) {
   // Pops x then y, returns y in *a, x in *b (so a op b is natural order)
   double x, y;
   if (!stack_pop(st, &x)) return false;
   if (!stack_pop(st, &y)) return false;
   *a = y;
   *b = x;
   return true;
}

void evaluate_tokens(const Lexer* lexer, Stack* stack, GC_String* err) {
   for (uint i = 0; i < lexer->tokens_len; ++i) {
      Token token = lexer->tokens[i];
      switch (token.type) {
      case NUMBER:
         stack_push(stack, atof(token.literal.data));
         break;

      case PLUS: {
         double a, b;
         if (!binop_pop2(stack, &a, &b)) goto underflow;
         stack_push(stack, a + b);
      } break;

      case MINUS: {
         double a, b;
         if (!binop_pop2(stack, &a, &b)) goto underflow;
         stack_push(stack, a - b);
      } break;

      case ASTRIX: {
         double a, b;
         if (!binop_pop2(stack, &a, &b)) goto underflow;
         stack_push(stack, a * b);
      } break;

      case SLASH: {
         double a, b;
         if (!binop_pop2(stack, &a, &b)) goto underflow;
         // Optional: check divide-by-zero; here we mimic C (inf/nan)
         stack_push(stack, a / b);
      } break;

      case ILLEGAL:
      case EOF_:
      case WORD:
      case USCORE:
      case ASSIGN:
      case CARET:
      case PERCENT:
      case DOLLAR:
      case HASH:
      case BANG:
      case AT:
      case AND_:
      case PIPE:
      case TILDA:
      case BTICK:
      case QUESTION:
      case DOT:
      case LT:
      case GT:
      case EQ:
      case NOT_EQ_:
      case LT_EQ:
      case GT_EQ:
      case ARROW:
      case FAT_ARROW:
      case COMMA:
      case COLON:
      case SEMICOLON:
      case LPAREN:
      case RPAREN:
      case LBRACE:
      case RBRACE:
      case LBRACKET:
      case RBRACKET:
      case DOUBLE_QUOTE:
      case QUOTE:
         break;
      }

      continue;

   underflow: {
      char buf[64];
      int  n = snprintf(buf, sizeof(buf), "Stack underflow while evaluating token at index %u\n", i);
      if (n < 0) return; // ignore on error
      if ((size_t)n >= sizeof(buf)) n = (int)(sizeof(buf) - 1);
      gc_string_append(err, buf, (uint)n);
      continue;
   }
   }
}

void str_append(char* str, uint* str_len, const char* str2, uint str2_len) {
   uint old_len = *str_len;
   *str_len += str2_len;
   str = (char*)GC_realloc(str, *str_len);
   memcpy(str + old_len, str2, str2_len);
}

/* ---------- Public API ---------- */

GC_String run_calculator(Lexer* lexer) {
   Stack     stack = { 0 };
   GC_String out;
   gc_string_init(&out);
   GC_String err;
   gc_string_init(&err);

   evaluate_tokens(lexer, &stack, &err);

   if (err.len > 0) {
      gc_string_append(&out, err.data, err.len);
   }

   for (uint i = 0; i < stack.len; ++i) {
      gc_string_appendf(&out, "%g\n", stack.data[i]);
   }
   return out;
}