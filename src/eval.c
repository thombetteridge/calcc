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

static Value stack_buffer[2048];

static void push_number(Stack* stack, double x)
{
   Value val = { .tag = V_NUMBER, .num = x };
   stack_push(stack, val);
}

static bool pop_number(Stack* stack, double* x)
{
   Value out = { 0 };
   stack_top(stack, &out);
   if (out.tag == V_NUMBER) {
      if (stack_pop(stack, &out)) {
         *x = out.num;
         return true;
      }
   }
   return false;
}

static bool pop_number_2(Stack* stack, double* x, double* y)
{
   Value out_x = { 0 };
   Value out_y = { 0 };
   if (stack_pop_2(stack, &out_y, &out_x)) {
      if (out_x.tag == V_NUMBER && out_y.tag == V_NUMBER) {
         *x = out_x.num;
         *y = out_y.num;
         return true;
      }
   }
   return false;
}

// static void push_quote(Stack* stack, Quote quote)
// {
//    Value val = { .tag = V_QUOTE, .quote = quote };
//    stack_push(stack, val);
// }

static void print_tokens(Token* tokens, size_t len)
{
   for (size_t i = 0; i < len; ++i) {
      printf("%.*s ", (int)tokens[i].literal.len, tokens[i].literal.data);
   }
   printf("\n");
}

void evaluate_tokens(const Token* tokens, size_t tokens_len, Stack* stack)
{
   for (size_t i = 0; i < tokens_len; ++i) {
      Token token = tokens[i];
      switch (token.type) {
      case NUMBER:
         push_number(stack, atof(token.literal.data));
         break;

      case PLUS: {
         double x, y;
         if (!pop_number_2(stack, &x, &y)) {
            goto underflow;
         }
         push_number(stack, y + x);
      } break;

      case MINUS: {
         double x, y;
         if (!pop_number_2(stack, &x, &y)) {
            goto underflow;
         }
         push_number(stack, y - x);
      } break;

      case ASTRIX: {
         double x, y;
         if (!pop_number_2(stack, &x, &y)) {
            goto underflow;
         }
         push_number(stack, y * x);
      } break;

      case SLASH: {
         double x, y;
         if (!pop_number_2(stack, &x, &y)) {
            goto underflow;
         }
         push_number(stack, y / x);
      } break;

      case ILLEGAL:
      case EOF_:
      case WORD: {
         Keyword f;
         if ((f = keywords_table_get(&keywords, token.literal.data, token.literal.len))) {
            f(stack);
            break;
         }
         Token_Array* user_word = NULL;
         if ((user_word = user_words_table_get(&user_words, &token.literal))) {
            evaluate_tokens(user_word->data, user_word->len, stack);
            break;
         }

         Value* variable = NULL;
         if ((variable = variable_table_get(&variables, &token.literal))) {
            if (variable->tag == V_NUMBER) {
               push_number(stack, variable->num);
               break;
            }
            else if (variable->tag == V_QUOTE) {
               // print_tokens(variable->quote.data, variable->quote.len);
               evaluate_tokens(variable->quote.data, variable->quote.len, stack);
               break;
            }
            break;
         }

         // Report unknown word
         char buf[64];
         int  n = snprintf(buf, sizeof(buf), "Unknown word: '%s' at index %u\n", token.literal.data, (u32)i);
         if (n < 0) {
            return; // ignore on error
         }
         if ((size_t)n >= sizeof(buf)) {
            n = (int)(sizeof(buf) - 1);
         }
         string_builder_append(&error, buf, (size_t)n);
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
            string_builder_append(&error, buf, (size_t)n);
            break;
         }
         Value val;
         if (stack_pop(stack, &val)) {
            // var name is next token
            variable_table_add(&variables, tokens[++i].literal, val);
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
            string_builder_append(&error, buf, (size_t)n);
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
         String      word_name = { 0 };
         Token_Array definition;
         token_array_init(&definition);

         if (++i < tokens_len && tokens[i].type == WORD) {
            word_name = tokens[i].literal;
         }
         else {
            break;
         }

         while (++i < tokens_len && tokens[i].type != SEMICOLON) {
            token_array_push(&definition, tokens[i]);
         }

         if (tokens[i].type == SEMICOLON) {
            user_words_table_add(&user_words, word_name, definition);
         }
         break;
      case SEMICOLON: break;
      case LPAREN: break;
      case RPAREN: break;
      case LBRACE: {
         size_t start = i;
         size_t count = 0;
         while (++i < tokens_len && tokens[i].type != RBRACE) {
            ++count;
         }
         if (count == 0) {
            break;
         }
         if (tokens[i].type == RBRACE) {
            Token_Array quote_tokens;
            token_array_init(&quote_tokens);
            for (size_t j = 1; j <= count; ++j) {
               token_array_push(&quote_tokens, tokens[start + j]);
            }

            Quote quote_temp = { .data = quote_tokens.data, .len = quote_tokens.len };
            Value val        = { .tag = V_QUOTE, .quote = quote_clone_deep(&quote_temp) };
            stack_push(stack, val);
         }
         break;
      }
      case RBRACE: break;
      case LBRACKET: break;
      case RBRACKET: break;
      case DOUBLE_QUOTE: break;
      case QUOTE: break;
      }

         continue;

      underflow: {
         char buf[64];
         int  n = snprintf(buf, sizeof(buf), "Stack underflow while evaluating\ntoken at index %u\n", (u32)i);
         if (n < 0) {
            return; // ignore on error
         }
         if ((size_t)n >= sizeof(buf)) {
            n = (int)(sizeof(buf) - 1);
         }
         string_builder_append(&error, buf, (size_t)n);
         continue;
      }
      }
   }
}

/* trig */

static void calc_sin(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, sin(x));
   }
}

static void calc_cos(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, cos(x));
   }
}

static void calc_tan(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, tan(x));
   }
}

static void calc_asin(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, asin(x));
   }
}

static void calc_acos(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, acos(x));
   }
}

static void calc_atan(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, atan(x));
   }
}

/* math */

static void calc_sqrt(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, sqrt(x));
   }
}

static void calc_sq(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, x * x);
   }
}

static void calc_abs(Stack* stack)
{
   double x;
   if (pop_number(stack, &x)) {
      push_number(stack, fabs(x));
   }
}

/* constants */

static void calc_pi(Stack* stack)
{
   push_number(stack, 3.14159265358979323846);
}

/* stack ops */

static void calc_dup(Stack* s)
{
   Value val;
   if (stack_top(s, &val)) {
      stack_push(s, val);
   }
}

static void calc_swap(Stack* stack)
{
   double x, y;
   if (pop_number_2(stack, &y, &x)) {
      push_number(stack, x);
      push_number(stack, y);
   }
}

/* sequences */

static void calc_sum(Stack* stack)
{
   double result = 0;
   double x;
   while (pop_number(stack, &x)) {
      result += x;
   }
   push_number(stack, result);
}

static void calc_mean(Stack* stack)
{
   double len = (double)stack->len;
   if (len == 0) {
      return;
   }
   calc_sum(stack);
   double sum;
   pop_number(stack, &sum);
   push_number(stack, sum / len);
}

static void calc_range(Stack* stack)
{
   if (stack->len < 3) {
      return;
   }
   double step, end, start;

   pop_number(stack, &step);
   pop_number(stack, &end);
   pop_number(stack, &start);

   if (step == 0 || ((end - start) / step < 0)) {
      return;
   }

   size_t       failsafe = 0;
   size_t const fail_max = (size_t)1e6;
   if (step > 0) {
      for (double x = start; x < end; x += step) {
         push_number(stack, x);
         if (++failsafe > fail_max) {
            return;
         }
      }
   }
   else {
      for (double x = start; x > end; x += step) {
         push_number(stack, x);
         if (++failsafe > fail_max) {
            return;
         }
      }
   }
}

static void calc_iota(Stack* stack)
{
   double x;
   pop_number(stack, &x);
   int n = (int)x;
   for (int i = 1; i <= n; ++i) {
      push_number(stack, i);
   }
}

static void add_keywords(Keyword_Table* keywords_)
{
   keywords_table_add(keywords_, "sin", calc_sin);
   keywords_table_add(keywords_, "cos", calc_cos);
   keywords_table_add(keywords_, "tan", calc_tan);
   keywords_table_add(keywords_, "asin", calc_asin);
   keywords_table_add(keywords_, "acos", calc_acos);
   keywords_table_add(keywords_, "atan", calc_atan);

   keywords_table_add(keywords_, "sqrt", calc_sqrt);
   keywords_table_add(keywords_, "sq", calc_sq);
   keywords_table_add(keywords_, "abs", calc_abs);

   keywords_table_add(keywords_, "pi", calc_pi);

   keywords_table_add(keywords_, "dup", calc_dup);
   keywords_table_add(keywords_, "swap", calc_swap);

   keywords_table_add(keywords_, "sum", calc_sum);
   keywords_table_add(keywords_, "mean", calc_mean);
   keywords_table_add(keywords_, "range", calc_range);
   keywords_table_add(keywords_, "iota", calc_iota);
}

/* ---------- Public API ---------- */

void eval_init()
{
   keywords_table_init(&keywords);
   add_keywords(&keywords);
   user_words_table_init(&user_words);
   variable_table_init(&variables);
   error          = (String_Builder) { .data = error_string_buffer, .len = 0, .cap = sizeof(error_string_buffer) };
   output_builder = (String_Builder) { .data = output_string_buffer, .len = 0, .cap = sizeof(output_string_buffer) };
}

void eval_shutdown()
{
   // assert(false && "TODO");
   printf("eval_shutdown() does nothing\n");
}

String run_calculator(Lexer* lexer)
{
   Stack stack = { .data = stack_buffer, .len = 0, .cap = 2048 };

   string_builder_reset(&output_builder);
   string_builder_reset(&error);

   evaluate_tokens(lexer->tokens.data, lexer->tokens.len, &stack);

   if (error.len > 0) {
      string_builder_append(&output_builder, error.data, error.len);
   }

   for (size_t i = 0; i < stack.len; ++i) {
      char fmt_buffer[32];
      if (stack.data[i].tag == V_NUMBER) {
         int fmt_len = sprintf(fmt_buffer, "%g\n", stack.data[i].num);
         string_builder_append(&output_builder, fmt_buffer, fmt_len);
      }
      if (stack.data[i].tag == V_QUOTE) {
         int fmt_len = sprintf(fmt_buffer, "{ ... }\n");
         string_builder_append(&output_builder, fmt_buffer, fmt_len);
      }
   }

   string_builder_append(&output_builder, "\0", sizeof("\0"));
   output_builder.data[output_builder.cap - 1] = '\0'; // just incase

   String out = (String) { .data = output_builder.data, .len = output_builder.len };

   return out;
}
