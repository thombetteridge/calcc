#pragma once

#include "common.h"

typedef enum Token_Type {
  ILLEGAL,
  EOF_,

  NUMBER,

  KEYWORD,
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

  // DIAGRAPGHS
  LT,        // <
  GT,        // >
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
  uint       pos;
  char*      literal;
} Token;

typedef struct Lexer {
  char*  input;
  uint   input_len;
  Token* tokens;
  uint   tokens_len;
  uint   tokens_cap;
  uint   pos;
  uint   read_pos;
  char   ch;
} Lexer;

void lexer_feed(Lexer* lexer, char* input_, uint input_len_);
void lexer_run(Lexer* lexer) ;