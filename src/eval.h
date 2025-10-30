#pragma once

#include "lexer.h"

String run_calculator(Lexer* lexer);
void   eval_init();
void   eval_shutdown();