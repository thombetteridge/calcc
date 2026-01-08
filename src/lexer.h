#pragma once

#include "base.h"

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
