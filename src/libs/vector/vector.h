
#ifndef __VECTOR_HEADER__
#define __VECTOR_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct vector {
    void *ptr;
    size_t length;
    size_t capacity;
    size_t element_size;
};

enum vector_error {
    VEC_OK,
    VEC_MEM_ERR,
    VEC_OUT_OF_BOUNDS_ERR,
    VEC_NULL_PTR
};

const char *vector_error_desc(enum vector_error err);

enum vector_error vec_init(struct vector *v, size_t capacity, size_t element_size);

enum vector_error vec_push(struct vector *v, void *x);

enum vector_error vec_get(struct vector *v, void *x, size_t index);

enum vector_error vec_pop(struct vector *v, void *x);

void vec_deinit(struct vector *v, void (*destructor)(void*));

void *vec_get_unsafe(struct vector *v, size_t index);

void vec_empty(struct vector *v);






#endif
