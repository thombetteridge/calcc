#pragma once

#include "common.h"
#include "gcstructures.h"
#include "gcstring.h"



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
   uint       pos;
   GC_String  literal;
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
void lexer_init(Lexer* lexer);
void lexer_run(Lexer* lexer);