#pragma once

typedef unsigned char byte_t;

<<<<<<< Updated upstream
typedef unsigned char byte;

typedef struct Arena {
   byte*  buffer;
   size_t cap;
   size_t offset;
   size_t prev_offset;
} Arena;

Arena arena_new(byte* buffer, size_t buffer_size);
void* arena_alloc(Arena* a, size_t size);
void arena_pop(Arena* arena);
void arena_reset(Arena* arena);
=======
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
>>>>>>> Stashed changes
