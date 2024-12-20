/**
 * arena.h
 * Single-header Arena allocator library
 *
 * Author: Hossein Khosravi
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Definitions
 */

#ifndef _LIBARENA_ARENA_H_
#define _LIBARENA_ARENA_H_

/* check https://stackoverflow.com/a/8249232/19005972 */
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define LIBARENA_PLAT_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
    #define LIBARENA_PLAT_DARWIN
#elif defined(__linux__)
    #define LIBARENA_PLAT_LINUX
#elif defined(__FreeBSD__)
    #define LIBARENA_PLAT_FREEBSD
#else
    #define LIBARENA_PLAT_UNKNOWN
#endif /* platform */

#if defined(LIBARENA_PLAT_LINUX) || defined(LIBARENA_PLAT_FREEBSD)  \
    || defined(unix) || defined(__unix) || defined(__unix__) || defined(LIBARENA_PLAT_DARWIN)
    #define LIBARENA_PLAT_UNIX
#endif /* unix platform */

#if defined(LIBARENA_PLAT_UNKNOWN) && !defined(LIBARENA_PLAT_UNIX)
    #error "unknown platform"
#endif

#if defined(LIBARENA_PLAT_UNIX)
    #include <sys/mman.h>
#elif defined(LIBARENA_PLAT_WINDOWS)
    #include <Windows.h>
    #include <Memoryapi.h>
#endif

#define LIBARENA_DEFAULT_SIZE    (4096)

typedef struct __arena Arena_t;
Arena_t arena_new(unsigned long size);
void *arena_alloc(Arena_t *arena, unsigned long size);
void arena_clear(Arena_t *arena);
void arena_destroy(Arena_t *arena);

#endif /* _LIBARENA_ARENA_H_ */


/**
 * Implementation
 */

#ifdef LIBARENA_IMPLEMENTATION

#include <stdint.h>
#include <assert.h>

#ifndef NULL
    #define NULL ((void*)0)
#endif /* NULL */

#define LIBARENA_PTR_SIZE   (sizeof(void*))

#ifdef LIBARENA_PLAT_UNIX
    #define __ALLOC(size)   \
        mmap(NULL, (size), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0)
#else
    #define __ALLOC(size)   \
        VirtualAlloc(NULL, (size), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#endif

#ifdef LIBARENA_PLAT_UNIX
    #define __DEALLOC(p, size)  \
        munmap((p), (size));
#else
    #define __DEALLOC(p, size)  \
        VirtualFree((p), 0, MEM_RELEASE);
#endif


#define __next_buf_location_from_base(base, size) \
    ((uintptr_t*)((base) + (size) - LIBARENA_PTR_SIZE))

#define __next_buf_addr_from_base(base, size)   \
    ((void*)(*__next_buf_location_from_base(base, size)))


struct __arena {
    /* Pointer to first buffer */
    void *base;

    /* Pointer to top of buffer */
    void *ptr;

    /* End of buffer */
    void *end;

    /* Size of buffer */
    size_t size;
};

Arena_t arena_new(unsigned long size) {
    Arena_t a;
    a.size = size;
    a.base = __ALLOC(size);
    assert(a.base && "base buffer is NULL");
    a.ptr = a.base;
    a.end = __next_buf_location_from_base(a.base, a.size);
    *((uintptr_t*)a.end) = 0;
    return a;
}

void *arena_alloc(Arena_t *arena, unsigned long size) {
    /* Requested size is bigger than arena buffer size */
    if (size > arena->size)
        return NULL;

    if (arena->ptr + size >= arena->end) {
#ifdef LIBARENA_MULTI_BUFFER
        arena->ptr = __ALLOC(arena->size);
        assert(arena->ptr && "allocated buffer is NULL");
        *((uintptr_t*)arena->end) = (uintptr_t)arena->ptr;
        arena->end = __next_buf_location_from_base(arena->ptr, arena->size);
        *((uintptr_t*)arena->end) = 0;
#else
        return NULL;
#endif
    }
    void *p = arena->ptr;
    arena->ptr += size;
    return p;
}

static void __destroy_buffers(void *base, const size_t size) {
    if (!base)
        return;
    void *next = __next_buf_addr_from_base(base, size);
    __destroy_buffers(next, size);
    __DEALLOC(base, size);
}

void arena_clear(Arena_t *arena) {
    __destroy_buffers(__next_buf_addr_from_base(arena->base, arena->size), arena->size);
    arena->ptr = arena->base;
}

void arena_destroy(Arena_t *arena) {
    __destroy_buffers(arena->base, arena->size);
}

#endif /* LIBARENA_ARENA_IMPLEMENTATION */


#ifdef __cplusplus
}
#endif
