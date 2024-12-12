/**
 * arena.h
 * Single-header library to use fixed buffer memory allocator (Arena allocator).
 *
 * Author: Hossein Khosravi
 */

/**
 * TODO:
 *  - Extend buffers
 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Definitions
 */

#ifndef LIBARENA_ARENA_H
#define LIBARENA_ARENA_H

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
int arena_destroy(Arena_t *arena);

#endif /* LIBARENA_ARENA_H */


/**
 * Implementation
 */

#ifdef LIBARENA_ARENA_IMPLEMENTATION

#include <stdint.h>
#include <assert.h>

#ifndef NULL
    #define NULL ((void*)0)
#endif /* NULL */

#define PTR_SIZE    (sizeof(void*))

#ifdef LIBARENA_PLAT_UNIX
    #define __ALLOC(size)   \
        mmap(NULL, (size), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0)
#else
    #define __ALLOC(size)   \
        VirtualAlloc(NULL, (size), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#endif

#define __calc_free_space(ptr, end) ((size_t)((end) - (ptr) - 1))


struct __arena {
    /* Pointer to first buffer */
    void *base;

    /* Pointer to top of buffer */
    void *ptr;

    /* End of buffer */
    void *end;

    /* Remaining free bytes in current buffer */
    size_t free;

    /* Size of buffer */
    size_t size;
};

Arena_t arena_new(unsigned long size) {
    Arena_t a;
    a.base = __ALLOC(size);
    assert(a.base && "base buffer is NULL");
    a.ptr = a.base;
    a.end = a.base + size - PTR_SIZE;
    a.size = size;
    a.free = __calc_free_space(a.ptr, a.end);
    return a;
}

void *arena_alloc(Arena_t *arena, unsigned long size) {
    if (arena->ptr + size >= arena->end) {
#ifdef LIBARENA_MULTI_BUFFER
        arena->ptr = __ALLOC(arena->size);
        assert(arena->ptr && "allocated buffer is NULL");
        *((uintptr_t*)arena->end) = (uintptr_t)arena->ptr;
        arena->end = arena->ptr + arena->size - PTR_SIZE;
#else
        return NULL;
#endif
    }
    void *p = arena->ptr;
    arena->ptr += size;
    arena->free = __calc_free_space(arena->ptr, arena->end);
    return p;
}

void arena_clear(Arena_t *arena) {
    arena->ptr = arena->base;
}

int arena_destroy(Arena_t *arena) {
#ifdef LIBARENA_PLAT_UNIX
    return (!munmap(arena->base, arena->size));
#else
    return VirtualFree(arena->base, 0, MEM_RELEASE);
#endif
}

#endif /* LIBARENA_ARENA_IMPLEMENTATION */


#ifdef __cplusplus
}
#endif
