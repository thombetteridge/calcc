#include "arena.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


static ArenaRegion * ArenaRegion_new(size_t cap) {
    assert(cap > sizeof(ArenaRegion));
    ArenaRegion * region = malloc(cap);
    region->buffer       = (char *)(region + 1);
    region->offset       = 0;
    region->capacity     = cap - sizeof(ArenaRegion);
    region->next         = NULL;
    return region;
}


static size_t align_forward(size_t ptr, size_t align) {
    size_t const modulo = ptr & (align - 1);
    return modulo ? (ptr + (align - modulo)) : ptr;
}

void arena_reserve(Arena * a, size_t cap) {
    if (a->head == NULL) {
        ArenaRegion * node = ArenaRegion_new(cap);
        a->head            = node;
        return;
    }

    if (cap < a->head->capacity)
        return;

    size_t        new_cap = align_forward(cap, sizeof(void *));
    ArenaRegion * node    = ArenaRegion_new(new_cap);
    node->next            = a->head;
    a->head               = node;
}

void * arena_alloc(Arena * a, size_t bytes) {
    if (a->head == NULL) {
        ArenaRegion * node = ArenaRegion_new(DEFAULT_REGION_SIZE);
        a->head            = node;
    }

    if (a->head->offset + bytes > a->head->capacity) {
        size_t new_cap     = a->head->capacity * 2 > bytes ? a->head->capacity * 2 : bytes;
        new_cap            = align_forward(new_cap, sizeof(void *));
        ArenaRegion * node = ArenaRegion_new(new_cap);
        node->next         = a->head;
        a->head            = node;
    }

    void * ptr = a->head->buffer + a->head->offset;
    memset(ptr, 0, bytes);
    a->head->offset = align_forward(a->head->offset + bytes, sizeof(void *));
    return ptr;
}


void arena_clear(Arena * a) {
    if (a->head == NULL)
        return;

    ArenaRegion * node = a->head->next;
    while (node) {
        ArenaRegion * next = node->next;
        free(node);
        node = next;
    }

    a->head->offset = 0;
    a->head->next   = NULL;
}

void arena_destroy(Arena * a) {
    arena_clear(a);
    free(a->head);
    a->head = NULL;
}

ArenaMarker arena_mark(Arena * a) {
    return (ArenaMarker) { .parent = a->head, .offset = a->head->offset };
}

void arena_pop(Arena * a, ArenaMarker mark) {
    if (a->head != mark.parent)
        return;

    if (mark.offset > a->head->offset)
        return;

    a->head->offset = mark.offset;
}