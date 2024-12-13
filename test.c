#include <stdio.h>
#include <assert.h>

#define LIBARENA_ARENA_IMPLEMENTATION
#define LIBARENA_MULTI_BUFFER
#include "arena.h"

#define XS_LEN  (20)

int main(void) {
    int i;
    Arena_t a = arena_new(LIBARENA_DEFAULT_SIZE);
    assert(a.base != NULL && "buffer is NULL");

    int *xs = (int*)arena_alloc(&a, sizeof(int) * XS_LEN);
    assert(xs != NULL && "allocation for 10 ints returned NULL");


    for (i = 0; i < XS_LEN; i++)
        xs[i] = i;

    for (i = 0; i < XS_LEN; i++)
        printf("%d\n", xs[i]);

    arena_destroy(&a);
    return 0;
}
