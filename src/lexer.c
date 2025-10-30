#include "lexer.h"

#include <stdbool.h>
#include <stdlib.h>

/* fwd decs */
static void   read_char(Lexer* lexer);
static bool   is_letter(char c);
static bool   is_number(char c);
static char   peak(Lexer* lexer);
static String read_word(Lexer* lexer);
static String read_number(Lexer* lexer);
static void   read_comment(Lexer* lexer);
static Token  new_token(Token_Type type, Lexer* lexer);
static Token  new_token2(Token_Type type, Lexer* lexer);
static Token  new_word_token(Lexer* lexer);
static Token  new_number_token(Lexer* lexer);
static void   next_token(Lexer* lexer);

void token_array_init(Token_Array* arr)
{
   arr->len  = 0;
   arr->cap  = 8;
   arr->data = (Token*)malloc(arr->cap * sizeof(Token));
}

void token_array_push(Token_Array* arr, Token x)
{
   if (arr->len == arr->cap) {
      arr->cap *= 2;
      arr->data = (Token*)realloc(arr->data, sizeof(Token) * arr->cap);
   }
   arr->data[arr->len] = x;
   arr->len++;
}
void token_array_pop(Token_Array* arr)
{
   arr->len--;
}

void token_array_free(Token_Array* arr)
{
   free(arr->data);
}

void lexer_run(Lexer* lexer)
{
   while (lexer->pos < lexer->input.len) {
      next_token(lexer);
   }
}

void lexer_init(Lexer* lexer)
{
   lexer->pos      = 0;
   lexer->read_pos = 0;
   lexer->ch       = 0;
   token_array_init(&lexer->tokens);
}

void lexer_shutdown(Lexer* lexer)
{
   token_array_free(&lexer->tokens);
}

void lexer_feed(Lexer* lexer, char* input_, size_t input_len_)
{
   lexer->pos        = 0;
   lexer->read_pos   = 0;
   lexer->ch         = 0;
   lexer->input.data = input_;
   lexer->input.len  = input_len_;
   lexer->tokens.len = 0; // RESET THIS!
   read_char(lexer);
   lexer_run(lexer);
};

static void read_char(Lexer* lexer)
{
   if (lexer->read_pos >= lexer->input.len || lexer->input.data[lexer->read_pos] == '\0') {
      lexer->ch       = 0;
      lexer->pos      = lexer->input.len;
      lexer->read_pos = lexer->input.len;
   }
   else {
      lexer->ch  = lexer->input.data[lexer->read_pos];
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
   if (lexer->read_pos >= lexer->input.len) {
      result = 0;
   }
   else {
      result = lexer->input.data[lexer->read_pos];
   }
   return result;
}

static String read_word(Lexer* lexer)
{
   size_t start = lexer->pos;
   while (is_letter(lexer->ch) || is_number(lexer->ch)) {
      read_char(lexer);
   }
   if (lexer->ch != 0) {
      lexer->read_pos--;
   }
   size_t out_len = lexer->pos - start;
   return (String) { .data = lexer->input.data + start, .len = out_len };
}

static String read_number(Lexer* lexer)
{
   size_t start = lexer->pos;

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

   size_t out_len = lexer->pos - start;
   return (String) { .data = lexer->input.data + start, .len = out_len };
}

static void read_comment(Lexer* lexer)
{
   size_t count = 0; // backup exit lol
   while (lexer->ch != '\n' && lexer->ch != '\r' && count < 120) {
      read_char(lexer);
      ++count;
   }
}

static Token new_token(Token_Type type, Lexer* lexer)
{
   String literal = (String) { .data = lexer->input.data + lexer->pos, .len = 1 };
   return (Token) {
      .type    = type,
      .pos     = lexer->pos,
      .literal = literal
   };
}

static Token new_token2(Token_Type type, Lexer* lexer)
{
   String str    = (String) { .data = lexer->input.data + lexer->pos, .len = 2 };
   Token  result = (Token) { .type = type, .pos = lexer->pos, .literal = str };
   lexer->read_pos++;
   return result;
}

static Token new_word_token(Lexer* lexer)
{
   size_t start = lexer->pos;
   String str   = read_word(lexer);
   return (Token) { .type = WORD, .pos = start, .literal = str };
}

static Token new_number_token(Lexer* lexer)
{
   size_t start = lexer->pos;
   String str   = read_number(lexer);
   return (Token) { .type = NUMBER, .pos = start, .literal = str };
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
         tok = new_token(ASSIGN, lexer);
      }
      break;
   case '+':
      tok = new_token(PLUS, lexer);
      break;
   case '-':
      if (peak(lexer) == '>') {
         tok = new_token2(ARROW, lexer);
      }
      else if (is_number(peak(lexer))) {
         tok = new_number_token(lexer);
      }
      else {
         tok = new_token(MINUS, lexer);
      }
      break;
   case '*':
      tok = new_token(ASTRIX, lexer);
      break;
   case '/':
      tok = new_token(SLASH, lexer);
      break;
   case '^':
      tok = new_token(CARET, lexer);
      break;
   case '%':
      tok = new_token(PERCENT, lexer);
      break;
   case '$':
      tok = new_token(DOLLAR, lexer);
      break;
   case '#':
      // --- COMMENT ---
      read_comment(lexer); // read to newline
      read_char(lexer);    // bump pointer past newline
      return;
      // tok = new_token(HASH,lexer);
      break;
   case '.':
      if (is_number(peak(lexer))) {
         tok = new_number_token(lexer);
      }
      else {
         tok = new_token(DOT, lexer);
      }
      break;
   case '!':
      if (peak(lexer) == '=') {
         tok = new_token2(NOT_EQ_, lexer);
      }
      else {
         tok = new_token(BANG, lexer);
      }
      break;
   case '@':
      tok = new_token(AT, lexer);
      break;
   case '&':
      tok = new_token(AND_, lexer);
      break;
   case '|':
      tok = new_token(PIPE, lexer);
      break;
   case '~':
      tok = new_token(TILDA, lexer);
      break;
   case '`':
      tok = new_token(BTICK, lexer);
      break;
   case '<':
      if (peak(lexer) == '=') {
         tok = new_token2(LT_EQ, lexer);
      }
      else {
         tok = new_token(LT, lexer);
      }
      break;
   case '>':
      if (peak(lexer) == '=') {
         tok = new_token2(GT_EQ, lexer);
      }
      else {
         tok = new_token(GT, lexer);
      }
      break;
   case '?':
      tok = new_token(QUESTION, lexer);
      break;
   case ';':
      tok = new_token(SEMICOLON, lexer);
      break;
   case ':':
      tok = new_token(COLON, lexer);
      break;
   case '{':
      tok = new_token(LBRACE, lexer);
      break;
   case '}':
      tok = new_token(RBRACE, lexer);
      break;
   case '[':
      tok = new_token(LBRACKET, lexer);
      break;
   case ']':
      tok = new_token(RBRACKET, lexer);
      break;
   case '(':
      tok = new_token(LPAREN, lexer);
      break;
   case ')':
      tok = new_token(RPAREN, lexer);
      break;
   case '_': {
      tok = new_token(USCORE, lexer);
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
         tok = new_token(ILLEGAL, lexer);
      }
      break;
   }
   read_char(lexer);
   token_array_push(&lexer->tokens, tok);
}
