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

#ifndef NULL
    #define NULL ((void*)0)
#endif /* NULL */
#define PTR_SIZE    (sizeof(void*))

struct __arena {
    /* Buffer */
    void *buf;

    /* Pointer to top of buffer */
    void *ptr;

    /* End of buffer */
    void *end;
};

Arena_t arena_new(unsigned long size) {
    Arena_t a;
#ifdef LIBARENA_PLAT_UNIX
    a.buf = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
#else
    a.buf = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#endif
    a.ptr = a.buf;
    a.end = a.buf + size;
    return a;
}

void *arena_alloc(Arena_t *arena, unsigned long size) {
    void *p = arena->ptr;
    /* TODO: At this time the buffer is not growable.
     * Handle this edge cases by allocating new buffer and extending
     * original buffer by storing new buffer's address in last bytes
     * of original buffer */
    if ((p + size) >= (arena->end - PTR_SIZE - 1))
        return NULL;
    arena->ptr += size;
    return p;
}

void arena_clear(Arena_t *arena) {
    arena->ptr = arena->buf;
}

int arena_destroy(Arena_t *arena) {
#ifdef LIBARENA_PLAT_UNIX
    return (!munmap(arena->buf, (unsigned long)arena->end-(unsigned long)arena->buf));
#else
    return VirtualFree(arena->buf, 0, MEM_RELEASE);
#endif
}

#endif /* LIBARENA_ARENA_IMPLEMENTATION */


#ifdef __cplusplus
}
#endif
