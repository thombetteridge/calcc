#include "eval.h"

#include "common.h"
#include "gcstructures.h"
#include "lexer.h"

#include "gc/gc.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GC_Table keywords;

void evaluate_tokens(const Lexer* lexer, Stack* stack, GC_String* err)
{
   for (uint i = 0; i < lexer->tokens_len; ++i) {
      Token token = lexer->tokens[i];
      switch (token.type) {
      case NUMBER:
         stack_push(stack, atof(token.literal.data));
         break;

      case PLUS: {
         double a, b;
         if (!stack_pop_2(stack, &a, &b)) {
            goto underflow;
         }
         stack_push(stack, a + b);
      } break;

      case MINUS: {
         double a, b;
         if (!stack_pop_2(stack, &a, &b)) {
            goto underflow;
         }
         stack_push(stack, a - b);
      } break;

      case ASTRIX: {
         double a, b;
         if (!stack_pop_2(stack, &a, &b)) {
            goto underflow;
         }
         stack_push(stack, a * b);
      } break;

      case SLASH: {
         double a, b;
         if (!stack_pop_2(stack, &a, &b)) {
            goto underflow;
         }
         // Optional: check divide-by-zero; here we mimic C (inf/nan)
         stack_push(stack, a / b);
      } break;

      case ILLEGAL:
      case EOF_:
      case WORD: {
         GC_Func f;
         if ((f = gc_table_find(&keywords, token.literal.data))) {
            f(stack);
         }
      } break;

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
      if (n < 0) {
         return; // ignore on error
      }
      if ((size_t)n >= sizeof(buf)) {
         n = (int)(sizeof(buf) - 1);
      }
      gc_string_append(err, buf, (uint)n);
      continue;
   }
   }
}

void calc_sin(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, sin(x));
   }
}

void calc_cos(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, cos(x));
   }
}

void calc_tan(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, tan(x));
   }
}

void calc_sqrt(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, sqrt(x));
   }
}

void calc_sq(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, x * x);
   }
}

void calc_pi(Stack* stack)
{
   stack_push(stack, 3.14159265358979323846);
}

static void add_keywords(GC_Table* keywords)
{
   gc_table_add(keywords, "sin", calc_sin);
   gc_table_add(keywords, "cos", calc_cos);
   gc_table_add(keywords, "tan", calc_tan);
   gc_table_add(keywords, "sqrt", calc_sqrt);
   gc_table_add(keywords, "sq", calc_sq);
   gc_table_add(keywords, "pi", calc_pi);
}

void str_append(char* str, uint* str_len, const char* str2, uint str2_len)
{
   uint old_len = *str_len;
   *str_len += str2_len;
   str = (char*)GC_realloc(str, *str_len);
   memcpy(str + old_len, str2, str2_len);
}

/* ---------- Public API ---------- */

GC_String run_calculator(Lexer* lexer)
{
   static bool needs_to_init_tables = true;

   if (needs_to_init_tables) {
      gc_table_init(&keywords);
      add_keywords(&keywords);

      needs_to_init_tables = false;
   }

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