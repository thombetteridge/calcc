#pragma once

typedef unsigned char byte_t;

typedef struct Arena Arena;
struct Arena {
   byte_t *ptr;
   size_t  len, cap;
   size_t  prev_offset;
};

void   arena_init(Arena *a, byte_t *buffer, size_t buffer_size);
void  *arena_alloc(Arena *a, size_t size);
size_t arena_mark(Arena a);
void   arena_pop(Arena *a, size_t mark);
void   arena_reset(Arena *a);
