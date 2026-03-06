
#ifndef __HASHMAP_HEADER__
#define __HASHMAP_HEADER__



#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "libs/arena/arena.h"

enum hashmap_result {
    HMAP_OK,
    HMAP_MEM_ERR,
    HMAP_NO_ENTRY,
    
};




struct hashmap_item {
    struct hashmap_item *next;
    char *key;
    int value;
};

struct hashmap {
    struct hashmap_item **table;
    size_t size;
};


enum hashmap_result hashmap_init(struct hashmap *hm, size_t size);

enum hashmap_result hashmap_add( struct hashmap *hm, const char *key, int value);

enum hashmap_result hashmap_get(const struct hashmap *hm, const char *key, int *value);

enum hashmap_result hashmap_find(const struct hashmap *hm, const char *key);

enum hashmap_result hashmap_remove(struct hashmap *hm, const char *key);

void hashmap_deinit(struct hashmap *hm);

#endif
