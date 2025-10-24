#include "lexer.h"
#include "common.h"
#include "gcstring.h"

#include "gc/gc.h"

#include <stdbool.h>

/* fwd decs */
static void      read_char(Lexer* lexer);
static bool      is_letter(char c);
static bool      is_number(char c);
static char      peak(Lexer* lexer);
static GC_String read_word(Lexer* lexer);
static GC_String read_number(Lexer* lexer);
static void      read_comment(Lexer* lexer);
static Token     new_token(Token_Type type, char c, uint position);
static Token     new_token2(Token_Type type, Lexer* lexer);
static Token     new_word_token(Lexer* lexer);
static Token     new_number_token(Lexer* lexer);
static void      next_token(Lexer* lexer);

void lexer_run(Lexer* lexer)
{
   while (lexer->pos < lexer->input_len) {
      next_token(lexer);
   }
}

void lexer_init(Lexer* lexer)
{
   lexer->pos        = 0;
   lexer->read_pos   = 0;
   lexer->ch         = 0;
   lexer->tokens_len = 0;
   lexer->tokens_cap = 128;
   lexer->tokens     = NEW(Token, lexer->tokens_cap);
}

void lexer_feed(Lexer* lexer, char* input_, uint input_len_)
{
   lexer->pos        = 0;
   lexer->read_pos   = 0;
   lexer->ch         = 0;
   lexer->input      = input_;
   lexer->input_len  = input_len_;
   lexer->tokens_len = 0; // RESET THIS!
   read_char(lexer);
   lexer_run(lexer);
};

static void read_char(Lexer* lexer)
{
   if (lexer->read_pos >= lexer->input_len || lexer->input[lexer->read_pos] == '\0') {
      lexer->ch       = 0;
      lexer->pos      = lexer->input_len;
      lexer->read_pos = lexer->input_len;
   }
   else {
      lexer->ch  = lexer->input[lexer->read_pos];
      lexer->pos = lexer->read_pos;
      lexer->read_pos++;
   }
}

static bool is_letter(char c)
{
   return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

static bool is_number(char c)
{
   return ('0' <= c && c <= '9');
}

static char peak(Lexer* lexer)
{
   char result;
   if (lexer->read_pos >= lexer->input_len) {
      result = 0;
   }
   else {
      result = lexer->input[lexer->read_pos];
   }
   return result;
}

static GC_String read_word(Lexer* lexer)
{
   GC_String result;
   gc_string_init(&result);
   uint start = lexer->pos;
   while (is_letter(lexer->ch) || is_number(lexer->ch)) {
      read_char(lexer);
   }
   if (lexer->ch != 0) {
      lexer->read_pos--;
   }
   uint out_len = lexer->pos - start;
   gc_string_append(&result, lexer->input + start, out_len);
   return result;
}

static GC_String read_number(Lexer* lexer)
{
   GC_String result;
   gc_string_init(&result);
   uint start = lexer->pos;

   if (lexer->ch == '-') {
      read_char(lexer);
   }

   bool has_e = false;
   for (;;) {
      if (is_number(lexer->ch) || lexer->ch == '.') {
         read_char(lexer);
      }
      else if (!has_e && (lexer->ch == 'e' || lexer->ch == 'E')) {
         has_e = true;
         read_char(lexer);
         if (lexer->ch == '+' || lexer->ch == '-') {
            read_char(lexer);
         }
      }
      else {
         break;
      }
   }

   if (lexer->ch != 0) {
      lexer->read_pos--;
   }

   uint out_len = lexer->pos - start;
   gc_string_append(&result, lexer->input + start, out_len);
   return result;
}

static void read_comment(Lexer* lexer)
{
   uint count = 0; // backup exit lol
   while (lexer->ch != '\n' && lexer->ch != '\r' && count < 120) {
      read_char(lexer);
      ++count;
   }
}

static Token new_token(Token_Type type, char c, uint position)
{
   GC_String str;
   gc_string_init(&str);
   char temp_str[2];
   temp_str[0] = c;
   temp_str[1] = '\0';
   gc_string_append(&str, temp_str, 2);
   return (Token) { .type = type, .pos = position, .literal = str };
}

static Token new_token2(Token_Type type, Lexer* lexer)
{
   GC_String str;
   gc_string_init(&str);
   char temp_str[3];
   temp_str[0] = *(lexer->input + lexer->pos);
   temp_str[1] = *(lexer->input + lexer->pos + 1);
   temp_str[2] = '\0';
   gc_string_append(&str, temp_str, 3);

   Token result = (Token) { .type = type, .pos = lexer->pos, .literal = str };
   lexer->read_pos++;
   return result;
}

static Token new_word_token(Lexer* lexer)
{
   GC_String str = read_word(lexer);
   return (Token) { .type = WORD, .pos = lexer->pos, .literal = str };
}

static Token new_number_token(Lexer* lexer)
{
   GC_String str = read_number(lexer);
   return (Token) { .type = NUMBER, .pos = lexer->pos, .literal = str };
}

static void push_token(Lexer* lexer, Token tok)
{
   if (lexer->tokens_cap == 0) {
      lexer->tokens_cap = 8;
      lexer->tokens     = NEW(Token, lexer->tokens_cap);
   }
   if (lexer->tokens_len == lexer->tokens_cap) {
      lexer->tokens_cap *= 2;
      lexer->tokens = RENEW(Token, lexer->tokens, lexer->tokens_cap);
   }
   lexer->tokens[lexer->tokens_len] = tok;
   lexer->tokens_len++;
}

static void next_token(Lexer* lexer)
{
   Token tok;
   switch (lexer->ch) {
   case ' ':
   case '\n':
   case '\t':
   case '\r':
      read_char(lexer); // skip whitespace
      return;
      break;
   case '=':
      if (peak(lexer) == '=') {
         tok = new_token2(EQ, lexer);
      }
      else if (peak(lexer) == '>') {
         tok = new_token2(FAT_ARROW, lexer);
      }
      else {
         tok = new_token(ASSIGN, lexer->ch, lexer->pos);
      }
      break;
   case '+':
      tok = new_token(PLUS, lexer->ch, lexer->pos);
      break;
   case '-':
      if (peak(lexer) == '>') {
         tok = new_token2(ARROW, lexer);
      }
      else if (is_number(peak(lexer))) {
         tok = new_number_token(lexer);
      }
      else {
         tok = new_token(MINUS, lexer->ch, lexer->pos);
      }
      break;
   case '*':
      tok = new_token(ASTRIX, lexer->ch, lexer->pos);
      break;
   case '/':
      tok = new_token(SLASH, lexer->ch, lexer->pos);
      break;
   case '^':
      tok = new_token(CARET, lexer->ch, lexer->pos);
      break;
   case '%':
      tok = new_token(PERCENT, lexer->ch, lexer->pos);
      break;
   case '$':
      tok = new_token(DOLLAR, lexer->ch, lexer->pos);
      break;
   case '#':
      tok = new_token(HASH, lexer->ch, lexer->pos);
      break;
   case '.':
      if (is_number(peak(lexer))) {
         tok = new_number_token(lexer);
      }
      else {
         tok = new_token(DOT, lexer->ch, lexer->pos);
      }
      break;
   case '!':
      if (peak(lexer) == '=') {
         tok = new_token2(NOT_EQ_, lexer);
      }
      else {
         tok = new_token(BANG, lexer->ch, lexer->pos);
      }
      break;
   case '@':
      tok = new_token(AT, lexer->ch, lexer->pos);
      break;
   case '&':
      tok = new_token(AND_, lexer->ch, lexer->pos);
      break;
   case '|':
      tok = new_token(PIPE, lexer->ch, lexer->pos);
      break;
   case '~':
      tok = new_token(TILDA, lexer->ch, lexer->pos);
      break;
   case '`':
      tok = new_token(BTICK, lexer->ch, lexer->pos);
      break;
   case '<':
      if (peak(lexer) == '=') {
         tok = new_token2(LT_EQ, lexer);
      }
      else {
         tok = new_token(LT, lexer->ch, lexer->pos);
      }
      break;
   case '>':
      if (peak(lexer) == '=') {
         tok = new_token2(GT_EQ, lexer);
      }
      else {
         tok = new_token(GT, lexer->ch, lexer->pos);
      }
      break;
   case '?':
      tok = new_token(QUESTION, lexer->ch, lexer->pos);
      break;
   case ';':
      // --- COMMENT ---
      read_comment(lexer); // read to newline
      read_char(lexer);    // bump pointer past newline
      return;
      // tok = new_token(semicolon, lexer->ch, lexer->pos);
      // break;
   case ':':
      tok = new_token(COLON, lexer->ch, lexer->pos);
      break;
   case '{':
      tok = new_token(LBRACE, lexer->ch, lexer->pos);
      break;
   case '}':
      tok = new_token(RBRACE, lexer->ch, lexer->pos);
      break;
   case '[':
      tok = new_token(LBRACKET, lexer->ch, lexer->pos);
      break;
   case ']':
      tok = new_token(RBRACKET, lexer->ch, lexer->pos);
      break;
   case '(':
      tok = new_token(LPAREN, lexer->ch, lexer->pos);
      break;
   case ')':
      tok = new_token(RPAREN, lexer->ch, lexer->pos);
      break;
   case '_': {
      tok = new_token(USCORE, lexer->ch, lexer->pos);
      break;
   }
   default:
      if (is_letter(lexer->ch)) {
         tok = new_word_token(lexer);
      }
      else if (is_number(lexer->ch)) {
         tok = new_number_token(lexer);
      }
      else {
         tok = new_token(ILLEGAL, lexer->ch, lexer->pos);
      }
      break;
   }
   read_char(lexer);
   push_token(lexer, tok);
}
