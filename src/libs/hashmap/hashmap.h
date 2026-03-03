
#ifndef __HASHMAP_HEADER__
#define __HASHMAP_HEADER__



#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

enum hashmap_result {
    HMAP_OK,
    HMAP_MEM_ERR,
    HMAP_NO_ENTRY,
    HMAP_FOUND
};




struct hashmap_item {
    struct hashmap_item *next;
    char *key;
    void *value;
};

struct hashmap {
    struct hashmap_item **table;
    size_t size;
    size_t value_size;
    void (*deinit)(void *value);
};


enum hashmap_result hashmap_init(struct hashmap *hm, size_t size, size_t value_size, void (*deinit)(void *value));

enum hashmap_result hashmap_add( struct hashmap *hm, const char *key, const void *value);

enum hashmap_result hashmap_get(const struct hashmap *hm, const char *key, void *value);

enum hashmap_result hashmap_find(const struct hashmap *hm, const char *key);

#endif
