
#include "vector.h"
#include <time.h>


const char *vector_error_desc(enum vector_error err) {
    switch (err) {
        case VEC_OK:
        return "Vector success.";

        case VEC_MEM_ERR:
        return "Vector memory error.";

        case VEC_OUT_OF_BOUNDS_ERR:
        return "Vector out of bounds error.";

        case VEC_NULL_PTR:
        return "Vector is uninitialised";

        default:
        return "Unknown error.";
    }
}

enum vector_error vec_init(struct vector *v, size_t capacity, size_t element_size) {
    v->capacity = capacity;
    v->element_size = element_size;
    v->length = 0;
    
    v->ptr = malloc(element_size * capacity);
    if (v->ptr == NULL) {
        return VEC_MEM_ERR;
    }

    return VEC_OK;
}

enum vector_error vec_push(struct vector *v, void *x) {
    if (v->ptr == NULL) {
        return VEC_NULL_PTR;
    }

    if (v->length >= v->capacity) {
        v->capacity = v->capacity * 2 + 1;
        void *tmp = realloc(v->ptr, v->element_size * v->capacity);
        if (tmp == NULL) {
            
            return VEC_MEM_ERR;
        }
        v->ptr = tmp;
    }
    memcpy((char*)v->ptr + (v->length * v->element_size), x, v->element_size);

    v->length += 1;
    return VEC_OK;
}


enum vector_error vec_get(const struct vector *v, void *x, size_t index) {
    if (v->ptr == NULL) {
        return VEC_NULL_PTR;
    }

    if (index >= v->length) {
        return VEC_OUT_OF_BOUNDS_ERR;
    }

    memcpy(x, (char*)v->ptr + (index * v->element_size), v->element_size);
    return VEC_OK;
}


enum vector_error vec_pop(struct vector *v, void *x) {
    if (v->ptr == NULL) {
        return VEC_NULL_PTR;
    }

    if (v->length == 0) {
        return VEC_OUT_OF_BOUNDS_ERR;
    }

    memcpy(x, (char*)v->ptr + ((v->length - 1) * v->element_size), v->element_size);
    v->length -= 1;
    return VEC_OK;

}


void *vec_get_ptr(const struct vector *v, size_t index) {
    return (char*)v->ptr + (index * v->element_size);
}

void vec_deinit(struct vector *v, void (*destructor)(void*)) {
    if (v->ptr == NULL) {
        return;
    }
    if (destructor != NULL) {
        for (size_t i = 0; i < v->length; i++) {
            destructor(vec_get_ptr(v, i));
        }
    }

    v->capacity = 0;
    v->element_size = 0;
    v->length = 0;
    free(v->ptr);
    v->ptr = NULL;
}

void vec_empty(struct vector *v) {
    v->length = 0;
}



void vec_init_u(struct vector *v, size_t capacity, size_t element_size) {
    if (vec_init(v, capacity, element_size) != VEC_OK) {
        printf("fatal error.");
        exit(1);
    }
}

void vec_push_u(struct vector *v, void *x) {
    if (vec_push(v, x) != VEC_OK) {
        printf("fatal error.");
        exit(1);
    }
}

void vec_get_u(const struct vector *v, void *x, size_t index) {
    if (vec_get(v, x, index) != VEC_OK) {
        printf("fatal error.");
        exit(1);
    }
}

void vec_pop_u(struct vector *v, void *x) {
    if (vec_push(v, x) != VEC_OK) {
        printf("fatal error.");
        exit(1);
    }
}


struct vector null_vector() {
    struct vector v;
    v.ptr = NULL;
    return v;
}
