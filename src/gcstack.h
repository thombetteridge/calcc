#pragma once

#include "common.h"

typedef struct Stack {
   double* data;
   uint    len;
   uint    cap;
} Stack;

void stack_push(Stack* stack, double x);
bool stack_top(const Stack* stack, double* out);
bool stack_pop(Stack* stack, double* out);
bool stack_pop_2(Stack* stack, double* y_out, double* x_out);
