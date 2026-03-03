
#ifndef __HASHSET_HEADER__
#define __HASHSET_HEADER__



#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

enum hashset_result {
    HMAP_OK,
    HMAP_MEM_ERR,
    HMAP_NO_ENTRY,
    HMAP_FOUND
};




struct hashset_item {
    struct hashset_item *next;
    char *key;
};

struct hashset {
    struct hashset_item **table;
    size_t size;
};


enum hashset_result hashset_init(struct hashset *hm, size_t size);

enum hashset_result hashset_add( struct hashset *hm, const char *key);

enum hashset_result hashset_get(const struct hashset *hm, const char *key);

enum hashset_result hashset_find(const struct hashset *hm, const char *key);

#endif
