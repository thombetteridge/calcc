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

static GC_Table            keywords;
static GC_Table_User_Words user_words;

void evaluate_tokens(const Token* tokens, uint tokens_len, Stack* stack, GC_String* err)
{
   for (uint i = 0; i < tokens_len; ++i) {
      Token token = tokens[i];
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
            break;
         }
         Token_Array* user_word = NULL;
         if (gc_user_table_find(&user_words, &token.literal, &user_word) && user_word) {
            evaluate_tokens(user_word->data, user_word->len, stack, err);
            break;
         }

         // Report unknown word
         char buf[64];
         int  n = snprintf(buf, sizeof(buf), "Unknown word: '%s' at index %u\n", token.literal.data, i);
         if (n < 0) {
            return; // ignore on error
         }
         if ((size_t)n >= sizeof(buf)) {
            n = (int)(sizeof(buf) - 1);
         }
         gc_string_append(err, buf, (uint)n);
         break;
      }

      case USCORE: break;
      case ASSIGN: break;
      case CARET: break;
      case PERCENT: break;
      case DOLLAR: break;
      case HASH: break;
      case BANG: break;
      case AT: break;
      case AND_: break;
      case PIPE: break;
      case TILDA: break;
      case BTICK: break;
      case QUESTION: break;
      case DOT: break;
      case LT: break;
      case GT: break;
      case EQ: break;
      case NOT_EQ_: break;
      case LT_EQ: break;
      case GT_EQ: break;
      case ARROW: break;
      case FAT_ARROW: break;
      case COMMA: break;
      case COLON: {
         GC_String word_name;
         gc_string_init(&word_name);
         Token_Array definition;
         token_array_init(&definition);

         if (++i < tokens_len && tokens[i].type == WORD) {
            gc_string_join(&word_name, &tokens[i].literal);
         }
         else {
            break;
         }

         while (++i < tokens_len && tokens[i].type != SEMICOLON) {
            token_array_push(&definition, tokens[i]);
         }

         gc_user_table_add(&user_words, word_name, definition);

      } break;
      case SEMICOLON: break;
      case LPAREN: break;
      case RPAREN: break;
      case LBRACE: break;
      case RBRACE: break;
      case LBRACKET: break;
      case RBRACKET: break;
      case DOUBLE_QUOTE: break;
      case QUOTE: break;
      }

      continue;

   underflow: {
      char buf[64];
      int  n = snprintf(buf, sizeof(buf), "Stack underflow while evaluating\ntoken at index %u\n", i);
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

/* trig */

static void calc_sin(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, sin(x));
   }
}

static void calc_cos(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, cos(x));
   }
}

static void calc_tan(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, tan(x));
   }
}

static void calc_asin(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, asin(x));
   }
}

static void calc_acos(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, acos(x));
   }
}

static void calc_atan(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, atan(x));
   }
}

/* math */

static void calc_sqrt(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, sqrt(x));
   }
}

static void calc_sq(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, x * x);
   }
}

static void calc_abs(Stack* stack)
{
   double x;
   if (stack_pop(stack, &x)) {
      stack_push(stack, fabs(x));
   }
}


/* constants */

static void calc_pi(Stack* stack)
{
   stack_push(stack, 3.14159265358979323846);
}

/* stack ops */

static void calc_dup(Stack* stack)
{
   double x;
   if (stack_top(stack, &x)) {
      stack_push(stack, x);
   }
}

static void calc_swap(Stack* stack)
{
   double x, y;
   if (stack_pop_2(stack, &y, &x)) {
      stack_push(stack, x);
      stack_push(stack, y);
   }
}

/* sequences */

static void calc_sum(Stack* stack)
{
   double result = 0;
   double x;
   while (stack_pop(stack, &x)) {
      result += x;
   }
   stack_push(stack, result);
}

static void calc_mean(Stack* stack)
{
   double len = stack->len;
   if (len == 0) {
      return;
   }
   calc_sum(stack);
   double sum;
   stack_pop(stack, &sum);
   stack_push(stack, sum / len);
}

static void calc_range(Stack* stack)
{
   if (stack->len < 3) {
      return;
   }
   double step, end, start;

   stack_pop(stack, &step);
   stack_pop(stack, &end);
   stack_pop(stack, &start);

   if (step == 0 || ((end - start) / step < 0)) {
      return;
   }

   uint       failsafe = 0;
   uint const fail_max = 1e6;
   if (step > 0) {
      for (double x = start; x < end; x += step) {
         stack_push(stack, x);
         if (++failsafe > fail_max) {
            return;
         }
      }
   }
   else {
      for (double x = start; x > end; x += step) {
         stack_push(stack, x);
         if (++failsafe > fail_max) {
            return;
         }
      }
   }
}

static void calc_iota(Stack* stack)
{
   double x;
   stack_pop(stack, &x);
   int n = (int)x;
   for (int i = 1; i <= n; ++i) {
      stack_push(stack, i);
   }
}

static void add_keywords(GC_Table* keywords)
{
   gc_table_add(keywords, "sin", calc_sin);
   gc_table_add(keywords, "cos", calc_cos);
   gc_table_add(keywords, "tan", calc_tan);
   gc_table_add(keywords, "asin", calc_asin);
   gc_table_add(keywords, "acos", calc_acos);
   gc_table_add(keywords, "atan", calc_atan);

   gc_table_add(keywords, "sqrt", calc_sqrt);
   gc_table_add(keywords, "sq", calc_sq);
   gc_table_add(keywords, "abs", calc_abs);

   gc_table_add(keywords, "pi", calc_pi);

   gc_table_add(keywords, "dup", calc_dup);
   gc_table_add(keywords, "swap", calc_swap);

   gc_table_add(keywords, "sum", calc_sum);
   gc_table_add(keywords, "mean", calc_mean);
   gc_table_add(keywords, "range", calc_range);
   gc_table_add(keywords, "iota", calc_iota);
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

      gc_user_table_init(&user_words);

      needs_to_init_tables = false;
   }

   Stack     stack = { 0 };
   GC_String out;
   gc_string_init(&out);
   GC_String err;
   gc_string_init(&err);

   evaluate_tokens(lexer->tokens, lexer->tokens_len, &stack, &err);

   if (err.len > 0) {
      gc_string_append(&out, err.data, err.len);
   }

   for (uint i = 0; i < stack.len; ++i) {
      gc_string_appendf(&out, "%g\n", stack.data[i]);
   }
   return out;
}