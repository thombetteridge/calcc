#include "eval.h"

#include "common.h"
#include "lexer.h"
#include "structures.h"
#include "tstring.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Keyword_Table    keywords;
static User_Words_Table user_words;
static Variable_Table   variables;

static String_Builder error;
static String_Builder output_builder;

static char error_string_buffer[KB(1)];
static char output_string_buffer[MB(2)];

// static char keywords_buffer[MB(1)];
// static char user_words_buffer[MB(1)];
// static char variables_buffer[MB(1)];

void evaluate_tokens(const Token* tokens, uint tokens_len, Stack* stack)
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
         Keyword f;
         if ((f = keyword_table_get(&keywords, token.literal.data))) {
            f(stack);
            break;
         }
         Token_Array* user_word = NULL;
         if ((user_word = user_words_table_get(&user_words, &token.literal))) {
            evaluate_tokens(user_word->data, user_word->len, stack);
            break;
         }

         double* variable = NULL;
         if ((variable = variable_table_get(&variables, &token.literal))) {
            stack_push(stack, *variable);
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
      case ASSIGN: {
         if (i + 1 >= tokens_len || tokens[i + 1].type != WORD) {
            char buf[64];
            int  n = snprintf(buf, sizeof(buf), "Error: '=' must be\nfollowed by variable name");
            if (n < 0) {
               return; // ignore on error
            }
            if ((size_t)n >= sizeof(buf)) {
               n = (int)(sizeof(buf) - 1);
            }
            gc_string_append(err, buf, (uint)n);
            break;
         }
         double x;
         if (stack_pop(stack, &x)) {
            String var_name;
            // gc_string_init(&var_name);
            var_name = gc_string_clone(&tokens[++i].literal);

            variable_table_add(&variables, var_name, x);
         }
         else {
            char buf[64];
            int  n = snprintf(buf, sizeof(buf), "Error: '=' underflow\n");
            if (n < 0) {
               return; // ignore on error
            }
            if ((size_t)n >= sizeof(buf)) {
               n = (int)(sizeof(buf) - 1);
            }
            gc_string_append(err, buf, (uint)n);
            break;
         }
         break;
      }

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
         String word_name;
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

         if (tokens[i].type == SEMICOLON) {
            gc_user_table_add(&user_words, word_name, definition);
         }

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
   uint const fail_max = (uint)1e6;
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

static void add_keywords(Keyword_Table* keywords_)
{
   keyword_table_add(keywords_, "sin", calc_sin);
   keyword_table_add(keywords_, "cos", calc_cos);
   keyword_table_add(keywords_, "tan", calc_tan);
   keyword_table_add(keywords_, "asin", calc_asin);
   keyword_table_add(keywords_, "acos", calc_acos);
   keyword_table_add(keywords_, "atan", calc_atan);

   keyword_table_add(keywords_, "sqrt", calc_sqrt);
   keyword_table_add(keywords_, "sq", calc_sq);
   keyword_table_add(keywords_, "abs", calc_abs);

   keyword_table_add(keywords_, "pi", calc_pi);

   keyword_table_add(keywords_, "dup", calc_dup);
   keyword_table_add(keywords_, "swap", calc_swap);

   keyword_table_add(keywords_, "sum", calc_sum);
   keyword_table_add(keywords_, "mean", calc_mean);
   keyword_table_add(keywords_, "range", calc_range);
   keyword_table_add(keywords_, "iota", calc_iota);
}

/* ---------- Public API ---------- */

void eval_init()
{
   keyword_table_init(&keywords);
   add_keywords(&keywords);
   user_words_table_init(&user_words);
   variable_table_init(&variables);
   error          = (String_Builder) { .data = error_string_buffer, .len = 0, .cap = sizeof(error_string_buffer) };
   output_builder = (String_Builder) { .data = output_string_buffer, .len = 0, .cap = sizeof(output_string_buffer) };
}

void eval_shutdown()
{
   assert(false && "TODO");
}

String run_calculator(Lexer* lexer)
{

   Stack  stack = { 0 };
   String err;

   string_builder_reset(&output_builder);
   string_builder_reset(&error);

   evaluate_tokens(lexer->tokens, lexer->tokens_len, &stack, &err);

   if (err.len > 0) {
      string_builder_append(&output_builder, err.data, err.len);
   }

   for (uint i = 0; i < stack.len; ++i) {
      char fmt_buffer[32];
      int  fmt_len = sprintf(fmt_buffer, "%g\n", stack.data[i]);
      string_builder_append(&output_builder, fmt_buffer, fmt_len);
   }

   string_builder_append(&output_builder, "\0", sizeof("\0"));
   output_builder.data[output_builder.cap - 1] = '\0'; // just incase

   String out = (String) { .data = output_builder.data, .len = output_builder.len };

   return out;
}