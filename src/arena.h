#pragma once

typedef unsigned char byte_t;

typedef struct arena arena_t;
struct arena {
   byte_t *ptr;
   size_t  len, cap;
   size_t  prev_offset;
};

void   arena_init(arena_t *a, byte_t *buffer, size_t buffer_size);
void  *arena_alloc(arena_t *a, size_t size);
size_t arena_mark(arena_t a);
void   arena_pop(arena_t *a, size_t mark);
void   arena_reset(arena_t *a);
