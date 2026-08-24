#include "base.h"

////////////////////////////////////
// Allocators
////////////////////////////////////

typedef struct {
    size_t total_allocated; // bytes currently outstanding
    size_t total_freed;     // bytes freed, cumulative
    size_t alloc_count;     // number of alloc() calls
    size_t dealloc_count;   // number of dealloc() calls
    size_t peak_allocated;  // high-water mark
} DefaultAllocatorStats;

static void * default_alloc(Allocator * self, size_t size, size_t alignment)
{
    DefaultAllocatorStats * stats = self->ctx;
    (void)alignment;

    void * buffer = malloc(size);

    if (buffer) {
        memset(buffer, 0, size);

        stats->total_allocated += size;
        stats->alloc_count += 1;

        if (stats->total_allocated - stats->total_freed > stats->peak_allocated) {
            stats->peak_allocated = stats->total_allocated - stats->total_freed;
        }
    }
    return buffer;
}

static void default_dealloc(Allocator * self, void * ptr, size_t size)
{
    DefaultAllocatorStats * stats = self->ctx;
    stats->total_freed += size;
    stats->dealloc_count += 1;
    free(ptr);
}

Allocator default_allocator_init(void)
{
    DefaultAllocatorStats * stats = malloc(sizeof(DefaultAllocatorStats));
    *stats                        = (DefaultAllocatorStats) { 0 };

    return (Allocator) {
        .ctx     = stats,
        .alloc   = default_alloc,
        .dealloc = default_dealloc,
    };
}

void default_allocator_report(Allocator * self)
{
    DefaultAllocatorStats * stats = self->ctx;

    fprintf(stderr,
        "total_allocated: %zu\n"
        "total_freed: %zu\n"
        "alloc_count: %zu\n"
        "dealloc_count: %zu\n"
        "peak_allocated: %zu\n",
        stats->total_allocated,
        stats->total_freed,
        stats->alloc_count,
        stats->dealloc_count,
        stats->peak_allocated);
}

void default_allocator_deinit(Allocator * self)
{
    default_allocator_report(self);
    free(self->ctx);
}

static void * fixed_alloc(Allocator * self, size_t size, size_t alignment)
{
    FixedAllocator * a       = (FixedAllocator *)self->ctx;
    size_t           aligned = (a->offset + alignment - 1) & ~(alignment - 1);
    if (aligned + size > a->capacity) {
        return NULL;
    }
    void * ptr = a->buffer + aligned;
    a->offset  = aligned + size;

    memset(ptr, 0, size);

    return ptr;
}

static void fixed_dealloc(Allocator * self, void * ptr, size_t size)
{
    (void)self->ctx;
    (void)ptr;
    (void)size;
}

Allocator fixed_allocator_init(uint8_t * buffer, size_t buffer_size)
{
    FixedAllocator * a          = (FixedAllocator *)buffer;
    size_t           header_end = sizeof(FixedAllocator);
    size_t           alignment  = alignof(max_align_t);
    size_t           aligned    = (header_end + alignment - 1) & ~(alignment - 1);

    a->buffer   = buffer + aligned;
    a->capacity = buffer_size - aligned;
    a->offset   = 0;
    return (Allocator) { .ctx = a, .alloc = fixed_alloc, .dealloc = fixed_dealloc };
}

void fixed_allocator_deinit(Allocator * self)
{
    *self = (Allocator) { 0 };
}


////////////////////////////////////
// Hash Table
////////////////////////////////////


size_t ht_hash37(char const * key)
{
    size_t hash = 0;

    for (size_t i = 0; key[i] != '\0'; i += 1) {
        hash = hash * 37 + (uint8_t)key[i];
    }

    return hash;
}

bool ht_key_eq(char const * a, char const * b)
{
    return (strcmp(a, b) == 0);
}

char * ht_key_dup(char const * s, Allocator * a)
{
    size_t len    = strlen(s);
    char * buffer = a->alloc(a, len + 1, alignof(char));
    memcpy(buffer, s, len);
    buffer[len] = '\0';

    return buffer;
}