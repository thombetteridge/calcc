#ifndef BASE_H
#define BASE_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef int8_t   i8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint64_t u64;
typedef int64_t  i64;

typedef size_t    usize;
typedef ptrdiff_t isize;

typedef float  f32;
typedef double f64;

typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

static_assert(sizeof(u8) == sizeof(i8), "");
static_assert(sizeof(u16) == sizeof(i16), "");
static_assert(sizeof(u32) == sizeof(i32), "");
static_assert(sizeof(u64) == sizeof(i64), "");
static_assert(sizeof(usize) == sizeof(isize), "");

static_assert(sizeof(bool) == 1, "");
static_assert(sizeof(u8) == 1, "");
static_assert(sizeof(u16) == 2, "");
static_assert(sizeof(u32) == 4, "");
static_assert(sizeof(u64) == 8, "");
static_assert(sizeof(f32) == 4, "");
static_assert(sizeof(f64) == 8, "");

/* NULL value. */
#define nil ((void *)0)

#define UNUSED(... /* x */) (void)(__VA_ARGS__)


#define static_assert_same_type(T1, T2)                                          \
    _Static_assert(__builtin_types_compatible_p(__typeof__(T1), __typeof__(T2)), \
        #T1                                                                      \
        " and " #T2                                                              \
        " must be the same type")

#define Swap(a, b)                 \
    do {                           \
        __typeof__(*(a)) t = *(a); \
        *(a)               = *(b); \
        *(b)               = t;    \
    } while (0)


#if defined(_MSC_VER) && !defined(_MAX_ALIGN_T_DEFINED) && !defined(__CLANG_MAX_ALIGN_T_DEFINED)
#define _MAX_ALIGN_T_DEFINED
typedef union {
    long long   ll;
    long double ld;
    void *      p;
} max_align_t;
#endif

#define Max(a, b) (a > b ? a : b)

#define Min(a, b) (a < b ? a : b)

#define Clamp(x, lo, hi) (Max(Min(x, hi), lo))


#define iterateEx(i, start, end) \
    usize i = (start);           \
    i < (end);                   \
    i += 1

#define iterate(i, n) iterateEx(i, 0, (n))

// DEBUG

#ifdef RELEASE

#define ENSURE(c)                    \
    do {                             \
        if (!(c))                    \
            __builtin_unreachable(); \
    } while (0)

#else

#define ENSURE(c)                                                                 \
    do {                                                                          \
        if (!(c)) {                                                               \
            fprintf(stderr, "\033[31;1mensure\033[0m(%s); \033[1m%s:%d\033[0m\n", \
                #c, __FILE__, __LINE__);                                          \
            abort();                                                              \
        }                                                                         \
    } while (0)

#endif // RELEASE

#define dump(fmt, ...)                                    \
    do {                                                  \
        fprintf(stderr, "\033[1m%s:%d\033[0m: " fmt "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__);           \
    } while (0)

#define UNIMPLEMENTED(...)                                           \
    do {                                                             \
        fprintf(stderr, "\033[1m%s:%d\033[0m: UNIMPLEMENTED: %s \n", \
            __FILE__, __LINE__, __VA_ARGS__);                        \
        abort();                                                     \
    } while (0)


#define TODO(...)                                           \
    do {                                                    \
        fprintf(stderr, "\033[1m%s:%d\033[0m: TODO: %s \n", \
            __FILE__, __LINE__, __VA_ARGS__);               \
        abort();                                            \
    } while (0)


// ALLOCATORS

// ABSTRACT CLASS
typedef struct Allocator Allocator;
struct Allocator {
    void * ctx;
    void * (*alloc)(Allocator * self, size_t size, size_t alignment);
    void (*dealloc)(Allocator * self, void * ptr, size_t size);
};

// DEFAULT (LIBC) ALLOCATOR
Allocator default_allocator_init(void);
void      default_allocator_deinit(Allocator * a);

// FIXED SIZE BUFFER ALLOCATOR
// the allocator stores this header at start of the buffer, takes 24 bytes (64bit);
typedef struct {
    uint8_t * buffer;
    size_t    offset;
    size_t    capacity;
} FixedAllocator;

Allocator fixed_allocator_init(uint8_t * buffer, size_t buffer_size);
void      fixed_allocator_deinit(Allocator * a);


#ifdef ALLOC_LOGGING

inline static void * alloc_logged(Allocator * allocator, size_t size, size_t align,
    char const * type_name, char const * file, int line)
{
    void * ptr = allocator->alloc(allocator, size, align);
    fprintf(stderr, "[ALLOC]   %s:%d  %zu bytes (align %zu)  %-12s -> %p\n",
        file, line, size, align, type_name, ptr);
    return ptr;
}

inline static void dealloc_logged(Allocator * allocator, void * ptr, size_t size,
    char const * file, int line)
{
    fprintf(stderr, "[DEALLOC] %s:%d  %zu bytes  ptr=%p\n", file, line, size, ptr);
    allocator->dealloc(allocator, ptr, size);
}

#define ALLOC(allocator, T, n) \
    alloc_logged((allocator), sizeof(T) * (n), alignof(T), #T, __FILE__, __LINE__)

#define DEALLOC(allocator, ptr, n) \
    dealloc_logged((allocator), (ptr), sizeof(__typeof__(*(ptr))) * (n), __FILE__, __LINE__)

#else

#define ALLOC(allocator, T, n)     (allocator)->alloc((allocator), sizeof(T) * (n), alignof(T))
#define DEALLOC(allocator, ptr, n) (allocator)->dealloc((allocator), (ptr), sizeof(__typeof__(*(ptr))) * (n))

#endif

#define arr_t(arr) __typeof__(*(arr)->ptr) // type helper

#define ArrayList(T)           \
    struct {                   \
        T *         ptr;       \
        size_t      len;       \
        size_t      cap;       \
        Allocator * allocator; \
    }

#define arr_init(arr, allocator_)        \
    do {                                 \
        (arr)->ptr       = NULL;         \
        (arr)->len       = 0;            \
        (arr)->cap       = 0;            \
        (arr)->allocator = (allocator_); \
    } while (0)

#define arr_deinit(arr)                                                                               \
    do {                                                                                              \
        if ((arr)->ptr)                                                                               \
            (arr)->allocator->dealloc((arr)->allocator, (arr)->ptr, sizeof(arr_t(arr)) * (arr)->cap); \
        (arr)->cap       = 0;                                                                         \
        (arr)->len       = 0;                                                                         \
        (arr)->ptr       = NULL;                                                                      \
        (arr)->allocator = NULL;                                                                      \
    } while (0)

#define arr_push(arr, value)                                                                              \
    do {                                                                                                  \
        if ((arr)->len == (arr)->cap) {                                                                   \
            size_t new_cap       = (arr)->cap ? (arr)->cap * 2 : 8;                                       \
            arr_t(arr) * new_ptr = (arr)->allocator->alloc((arr)->allocator,                              \
                sizeof(arr_t(arr)) * new_cap,                                                             \
                alignof(arr_t(arr)));                                                                     \
            if (!new_ptr) {                                                                               \
                fprintf(stderr, "%s : %d : ArrayList Allocation Failed\n", __FILE__, __LINE__);           \
                abort();                                                                                  \
            }                                                                                             \
            if ((arr)->ptr != NULL) {                                                                     \
                memcpy(new_ptr, (arr)->ptr, sizeof(arr_t(arr)) * (arr)->len);                             \
                (arr)->allocator->dealloc((arr)->allocator, (arr)->ptr, sizeof(arr_t(arr)) * (arr)->cap); \
            }                                                                                             \
            (arr)->ptr = new_ptr;                                                                         \
            (arr)->cap = new_cap;                                                                         \
        }                                                                                                 \
        (arr)->ptr[(arr)->len] = value;                                                                   \
        (arr)->len += 1;                                                                                  \
    } while (0)

#define arr_reserve(arr, n)                                                                           \
    do {                                                                                              \
        size_t req = (n);                                                                             \
        if (req <= (arr)->cap)                                                                        \
            break;                                                                                    \
        size_t new_cap = (arr)->cap ? (arr)->cap : 8;                                                 \
        while (new_cap < req)                                                                         \
            new_cap *= 2;                                                                             \
        arr_t(arr) * new_ptr = (arr)->allocator->alloc((arr)->allocator,                              \
            sizeof(arr_t(arr)) * new_cap,                                                             \
            alignof(arr_t(arr)));                                                                     \
        if (!new_ptr) {                                                                               \
            fprintf(stderr, "%s : %d : ArrayList Reserve Allocation Failed\n", __FILE__, __LINE__);   \
            abort();                                                                                  \
        }                                                                                             \
        if ((arr)->ptr != NULL) {                                                                     \
            memcpy(new_ptr, (arr)->ptr, sizeof(arr_t(arr)) * (arr)->len);                             \
            (arr)->allocator->dealloc((arr)->allocator, (arr)->ptr, sizeof(arr_t(arr)) * (arr)->cap); \
        }                                                                                             \
        (arr)->ptr = new_ptr;                                                                         \
        (arr)->cap = new_cap;                                                                         \
    } while (0)

#define arr_resize(arr, n)                                                                   \
    do {                                                                                     \
        size_t new_len = (n);                                                                \
        arr_reserve((arr), (new_len));                                                       \
        if (new_len > (arr)->len)                                                            \
            memset((arr)->ptr + (arr)->len, 0, (new_len - (arr)->len) * sizeof(arr_t(arr))); \
    } while (0)

#define arr_clear(arr)  \
    do {                \
        (arr)->len = 0; \
    } while (0)

#define arr_back(arr) ((arr)->ptr[(arr)->len - 1])

#define arr_front(arr) ((arr)->ptr[0])

#define arr_at(arr, idx) ((arr)->ptr[(idx)])

#define arr_sort(arr, fn)                                      \
    do {                                                       \
        qsort((arr)->ptr, (arr)->len, sizeof(arr_t(arr)), fn); \
    } while (0)

#define arr_each(arr, it)          \
    arr_t(arr) * it = (arr)->ptr;  \
    it != (arr)->ptr + (arr)->len; \
    it += 1

#define arr_pop(arr)            \
    do {                        \
        assert((arr)->len > 0); \
        (arr)->len -= 1;        \
    } while (0)

#define arr_erase(arr, idx)                                                                           \
    do {                                                                                              \
        assert(idx < (arr)->len);                                                                     \
        memmove((arr)->ptr + idx, (arr)->ptr + idx + 1, ((arr)->len - idx - 1) * sizeof(arr_t(arr))); \
        arr_pop(arr);                                                                                 \
    } while (0)

#define arr_erase_unstable(arr, idx)                  \
    do {                                              \
        assert(idx < (arr)->len);                     \
        (arr)->ptr[idx] = (arr)->ptr[(arr)->len - 1]; \
        arr_pop(arr);                                 \
    } while (0)


//// Table

size_t ht_hash37(char const * key);
bool   ht_key_eq(char const * a, char const * b);
char * ht_key_dup(char const * s, Allocator * a);

#define ht_t(ht) __typeof__(*(ht)->ptr) // type helper

#define HashTable(T)           \
    struct {                   \
        struct {               \
            char * key;        \
            T      value;      \
            bool   occupied;   \
        } * ptr;               \
                               \
        size_t      len;       \
        size_t      cap;       \
        Allocator * allocator; \
    }

#define ht_init(ht, init_cap, allocator_)                         \
    do {                                                          \
        (ht)->ptr       = NULL;                                   \
        (ht)->len       = 0;                                      \
        (ht)->cap       = init_cap;                               \
        (ht)->allocator = allocator_;                             \
        (ht)->ptr       = (ht)->allocator->alloc((ht)->allocator, \
            sizeof(ht_t(ht)) * init_cap,                          \
            alignof(ht_t(ht)));                                   \
    } while (0)

#define ht_deinit(ht)                                                       \
    do {                                                                    \
        for (size_t i = 0; i < (ht)->cap; i += 1) {                         \
            if ((ht)->ptr[i].occupied) {                                    \
                (ht)->allocator->dealloc((ht)->allocator, (ht)->ptr[i].key, \
                    strlen((ht)->ptr[i].key) + 1);                          \
            }                                                               \
        }                                                                   \
        if ((ht)->ptr)                                                      \
            (ht)->allocator->dealloc((ht)->allocator, (ht)->ptr,            \
                sizeof(ht_t(ht)) * (ht)->cap);                              \
        (ht)->cap       = 0;                                                \
        (ht)->len       = 0;                                                \
        (ht)->ptr       = NULL;                                             \
        (ht)->allocator = NULL;                                             \
    } while (0)

#define ht_insert_raw(ht, k, v)                                            \
    do {                                                                   \
        size_t h = ht_hash37(k) % (ht)->cap;                               \
        while ((ht)->ptr[h].occupied && !ht_key_eq(k, (ht)->ptr[h].key)) { \
            h = (h + 1) % (ht)->cap;                                       \
        }                                                                  \
        if (!(ht)->ptr[h].occupied) {                                      \
            (ht)->ptr[h].key      = ht_key_dup(k, (ht)->allocator);        \
            (ht)->ptr[h].value    = v;                                     \
            (ht)->ptr[h].occupied = true;                                  \
            (ht)->len += 1;                                                \
        }                                                                  \
        else {                                                             \
            (ht)->ptr[h].value = v;                                        \
        }                                                                  \
    } while (0)

#define ht_grow(ht, new_cap)                                                 \
    do {                                                                     \
        __typeof__(*(ht)) new_t = { 0 };                                     \
        ht_init(&new_t, (new_cap), (ht)->allocator);                         \
        for (size_t i = 0; i < (ht)->cap; i += 1) {                          \
            if ((ht)->ptr[i].occupied) {                                     \
                ht_insert_raw(&new_t, (ht)->ptr[i].key, (ht)->ptr[i].value); \
            }                                                                \
        }                                                                    \
        ht_deinit(ht);                                                       \
        (ht)->ptr       = new_t.ptr;                                         \
        (ht)->len       = new_t.len;                                         \
        (ht)->cap       = new_t.cap;                                         \
        (ht)->allocator = new_t.allocator;                                   \
    } while (0)

#define ht_insert(ht, k, v)                               \
    do {                                                  \
        if ((float)(ht)->len / (float)(ht)->cap > 0.7f) { \
            ht_grow(ht, (ht)->cap * 2);                   \
        }                                                 \
        ht_insert_raw(ht, k, v);                          \
    } while (0)


// GNU extension

#define ht_get(ht, k, v)                                                   \
    ({                                                                     \
        bool   result = false;                                             \
        size_t h      = ht_hash37(k) % (ht)->cap;                          \
        while ((ht)->ptr[h].occupied && !ht_key_eq(k, (ht)->ptr[h].key)) { \
            h = (h + 1) % (ht)->cap;                                       \
        }                                                                  \
        if (ht_key_eq(k, (ht)->ptr[h].key)) {                              \
            *v     = (ht)->ptr[h].value;                                   \
            result = true;                                                 \
        }                                                                  \
        result;                                                            \
    })


#endif
