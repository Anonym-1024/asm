
#ifndef __ARENA_HEADER__
#define __ARENA_HEADER__

#include <stdlib.h>
#include <errno.h>
#include <stdalign.h>
#include <stddef.h>




struct arena {
    void *ptr;
    size_t size;
    size_t offset;

    struct arena *next;
};

enum arena_result {
    ARENA_OK,
    ARENA_TOO_SMALL,
    ARENA_MEM_ERR
};


enum arena_result arena_init(struct arena *a, size_t size);

void *aralloc(struct arena *a, size_t size);

void arena_deinit(struct arena *a);



#endif
