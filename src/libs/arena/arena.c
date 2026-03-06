
#include "arena.h"


enum arena_result arena_init(struct arena *a, size_t size) {
    a->size = size;
    a->offset = 0;
    a->next = NULL;
    a->ptr = malloc(size);

    if (a->ptr == NULL) {
        return ARENA_MEM_ERR;
    }

    
    
    return ARENA_OK;
}

void *aralloc(struct arena *a, size_t size) {
    if (size > a->size) {
        errno = ENOMEM;
        return NULL;
    }

    
    size_t avail = a->size - a->offset;
    while (avail < size && a->next != NULL) {
        a = a->next;
        avail = a->size - a->offset;
    }

    if (avail >= size) {
        
        size_t alignment = alignof(max_align_t);
        size_t padding = (alignment - (a->offset % alignment)) % alignment;
        a->offset += padding;
        void *tmp = (char*)a->ptr + a->offset;
        a->offset += size;
        return tmp;
    }

    struct arena *new = malloc(sizeof(struct arena));
    if (new == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if (arena_init(new, a->size) != ARENA_OK) {
        free(new);
        errno = ENOMEM;
        return NULL;
    }

    a->next = new;
    size_t alignment = alignof(max_align_t);
    size_t padding = (alignment - (new->offset % alignment)) % alignment;
    new->offset += padding;
    void *tmp = (char*)new->ptr + new->offset;
    new->offset += size;
    return tmp;

    


}


void arena_deinit(struct arena *a) {
    if (a == NULL) {
        return ;
    }

    free(a->ptr);
    arena_deinit(a->next);
    free(a->next);
}

