#include "gcstack.h"

#include "gc/gc.h"

void stack_push(Stack* stack, double x)
{
   if (stack->cap == 0) {
      stack->cap  = 8;
      stack->data = (double*)GC_malloc(stack->cap * sizeof(double));
   }

   if (stack->len == stack->cap) {
      stack->cap *= 2;
      stack->data = (double*)GC_realloc(stack->data, stack->cap * sizeof(double));
   }

   stack->data[stack->len] = x;
   stack->len++;
}

bool stack_top(const Stack* stack, double* out)
{
   if (stack->len == 0) {
      return false;
   }
   *out = stack->data[stack->len - 1];
   return true;
}

bool stack_pop(Stack* stack, double* out)
{
   if (stack->len == 0) {
      return false;
   }
   stack_top(stack, out);
   stack->len--;
   return true;
}

bool stack_pop_2(Stack* stack, double* y_out, double* x_out)
{
   // Pops x then y, returns y in *a, x in *b (so a op b is natural order)
   double x, y;
   if (!stack_pop(stack, &x)) {
      return false;
   }
   if (!stack_pop(stack, &y)) {
      return false;
   }
   *y_out = y;
   *x_out = x;
   return true;
}
