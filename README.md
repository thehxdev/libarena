# Libarena
Easy to use, cross-platform and Single-header Arena allocator.

## API
Application programming interface for `libarena`.

### Include `arena.h`
Since the library is single-header, include `arena.h` file in **_JUST ONE SOURCE FILE_** with `LIBARENA_ARENA_IMPLEMENTATION` defined.
If you want Multi-Buffer support, define `LIBARENA_MULTI_BUFFER` too in that source file just **_once_**.
You can include `arena.h` in all source files. But the implementation details MUST be in one file.
```c
/* include the implementation */
#define LIBARENA_ARENA_IMPLEMENTATION
/* enable multi-buffer support */
#define LIBARENA_MULTI_BUFFER
#include "arena.h"
```
You can use multi-buffer mode if you want growable buffers. otherwise one fixed buffer will be used.

### Construct a new `Arena_t`
Construct a new `Arena_t` instance. This type holds and manages all information about your arena.
`arena_new` function accepts buffer's size. The default size is one page size (4096 bytes).
Since `libarena` uses `mmap` on \*nix systems and `VirtualAlloc` on windows, it's better to use the default size.
```c
Arena_t a = arena_new(LIBARENA_DEFAULT_SIZE);
```

### Allocate memory
Allocate on arena and get a pointer.
```c
/* allocate 10 integers */
int *xs = arena_alloc(&a, sizeof(int) * 10);
```

### Clear arena
Clear and reset arena.
```c
arena_clear(&a);
```

### Destroy arena
Destroy the arena and deallocate all allocated memory.
```c
arena_destroy(&a);
```
