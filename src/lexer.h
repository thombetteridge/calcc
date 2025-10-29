#pragma once

#include "tstring.h"

typedef enum Token_Type {
   ILLEGAL,
   EOF_,

   NUMBER,
   WORD,

   USCORE, // _

   // OPERATORS
   ASSIGN,   // =
   PLUS,     // +
   MINUS,    // -
   ASTRIX,   // *
   SLASH,    // /
   CARET,    // ^
   PERCENT,  // %
   DOLLAR,   // $
   HASH,     // #
   BANG,     // !
   AT,       // @
   AND_,     // &
   PIPE,     // |
   TILDA,    // ~
   BTICK,    // `
   QUESTION, // ?
   DOT,      // .
   LT,       // <
   GT,       // >

   // DIAGRAPGHS
   EQ,        // ==
   NOT_EQ_,   // !=
   LT_EQ,     // <=
   GT_EQ,     // >=
   ARROW,     // ->
   FAT_ARROW, // =>

   // DELIMINATORS
   COMMA,        // ,
   COLON,        // :
   SEMICOLON,    // ;
   LPAREN,       // (
   RPAREN,       // )
   LBRACE,       // {
   RBRACE,       // }
   LBRACKET,     // ]
   RBRACKET,     // [
   DOUBLE_QUOTE, // "
   QUOTE,        // '
} Token_Type;

typedef struct Token {
   Token_Type type;
   size_t     pos;
   String     literal;
} Token;

typedef struct Token_Array {
   Token* data;
   size_t len;
   size_t cap;
} Token_Array;

void token_array_init(Token_Array* arr);
void token_array_push(Token_Array* arr, Token x);
void token_array_pop(Token_Array* arr);
void token_array_free(Token_Array* arr);

typedef struct Lexer {
   String      input;
   Token_Array tokens;
   size_t      pos;
   size_t      read_pos;
   char        ch;
} Lexer;

void lexer_feed(Lexer* lexer, char* input_, size_t input_len_);
void lexer_init(Lexer* lexer);
void lexer_run(Lexer* lexer);
void lexer_shutdown(Lexer* lexer);
