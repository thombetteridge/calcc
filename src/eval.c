#include "common.h"
#include "gc/gc.h"
#include "lexer.h"

#include <stdlib.h>

typedef struct Stack {
    double* data;
    uint len;
    uint cap;
} Stack;

void stack_push(Stack* stack, double x)
{
    if (stack->cap == 0){
        stack->cap = 8;
        stack->data = (double*)GC_malloc_atomic(stack->cap * sizeof(double));
    }

    if (stack->len == stack->cap){
        stack->cap *= 2;
        stack->data = (double*)GC_realloc(stack->data, stack->cap * sizeof(double));
    }

    stack->data[stack->len] = x;
    stack->len++;
}

static Stack stack = {0};

void evaulate_tokens(const Lexer* lexer, char** result)
{
    for (uint i = 0; i < lexer->tokens_len; ++i) {
        Token token = lexer->tokens[i];

        switch (token.type) {

        case ILLEGAL:
            break;
        case EOF_:
            break;
        case NUMBER:
                stack_push(&stack,atof(token.literal));
            break;
        case KEYWORD:
            break;
        case WORD:
            break;
        case USCORE:
            break;
        case ASSIGN:
            break;
        case PLUS:
            break;
        case MINUS:
            break;
        case ASTRIX:
            break;
        case SLASH:
            break;
        case CARET:
            break;
        case PERCENT:
            break;
        case DOLLAR:
            break;
        case HASH:
            break;
        case BANG:
            break;
        case AT:
            break;
        case AND_:
            break;
        case PIPE:
            break;
        case TILDA:
            break;
        case BTICK:
            break;
        case QUESTION:
            break;
        case DOT:
            break;
        case LT:
            break;
        case GT:
            break;
        case EQ:
            break;
        case NOT_EQ_:
            break;
        case LT_EQ:
            break;
        case GT_EQ:
            break;
        case ARROW:
            break;
        case FAT_ARROW:
            break;
        case COMMA:
            break;
        case COLON:
            break;
        case SEMICOLON:
            break;
        case LPAREN:
            break;
        case RPAREN:
            break;
        case LBRACE:
            break;
        case RBRACE:
            break;
        case LBRACKET:
            break;
        case RBRACKET:
            break;
        case DOUBLE_QUOTE:
            break;
        case QUOTE:
            break;
        }
    }
}