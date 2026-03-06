
#include "hashmap.h"


static u_int64_t hash_64fnv(const void *data, size_t len) {
    const u_int8_t *bytes = (const u_int8_t *)data;

    // FNV-1a 64-bit constants
    uint64_t hash = 1469598103934665603ULL;  // offset basis
    const uint64_t prime = 1099511628211ULL; // FNV prime

    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)bytes[i];
        hash *= prime;
    }

    return hash;
}

static u_int64_t shash_64fnv(const char *str) {
    return hash_64fnv(str, strlen(str));
}



enum hashmap_result hashmap_init(struct hashmap *hm, size_t size) {
    
    hm->size = size;
    hm->table = malloc(sizeof(struct hashmap_item*) * size);
    if (hm->table == NULL) {
        return HMAP_MEM_ERR;
    }

    for (size_t i = 0; i < size; i++) {
        hm->table[i] = NULL;
    }
    return HMAP_OK;
}

static size_t get_index(const struct hashmap *hm, const char *key) {
    size_t hash = (size_t)shash_64fnv(key);
    return hash % hm->size;
}

enum hashmap_result hashmap_add(struct hashmap *hm, const char *key, int value) {
    size_t index = get_index(hm, key);
    struct hashmap_item *item = hm->table[index];

    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            
            item->value = value;
            return HMAP_OK;
        }
        item = item->next;
    }

    struct hashmap_item *new_item = malloc(sizeof(*new_item));
    if (new_item == NULL) {
        return HMAP_MEM_ERR;
    }

    new_item->key = malloc(strlen(key) + 1);
    if (new_item->key == NULL) {
        free(new_item);
        return HMAP_MEM_ERR;
    }
    strcpy(new_item->key, key);

    

    new_item->value = value;

    new_item->next = hm->table[index];
    hm->table[index] = new_item;

    return HMAP_OK;

}

enum hashmap_result hashmap_get(const struct hashmap *hm, const char *key, int *value) {
    struct hashmap_item *item = hm->table[get_index(hm, key)];

    

    while (item != NULL) {
        if (strcmp(key, item->key) == 0) {
            *value = item->value;
            return HMAP_OK;
        }
        item = item->next;
    }

    return HMAP_NO_ENTRY;


}

enum hashmap_result hashmap_find(const struct hashmap *hm, const char *key) {
    struct hashmap_item *item = hm->table[get_index(hm, key)];

    

    while (item != NULL) {
        if (strcmp(key, item->key) == 0) {
            return HMAP_OK;
        }
        item = item->next;
    }

    return HMAP_NO_ENTRY;
}



enum hashmap_result hashmap_remove(struct hashmap *hm, const char *key) {
    struct hashmap_item **item = &hm->table[get_index(hm, key)];

    while (*item != NULL) {

        if (strcmp((*item)->key, key) == 0) {
            struct hashmap_item *tmp = (*item)->next;
            
            free((*item)->key);
            
            free(*item);
            *item = tmp;
            return HMAP_OK;
        }
        item = &(*item)->next;
    }
    return HMAP_NO_ENTRY;
}


void hashmap_item_deinit(struct hashmap_item *item) {
    if (item->next != NULL) {
        hashmap_item_deinit(item->next);
        free(item->next);
    }
    free(item->key);
    
}

void hashmap_deinit(struct hashmap *hm) {
    struct hashmap_item *item;
    for (size_t i = 0; i < hm->size; i++) {
        item = hm->table[i];
        if (item != NULL) {
            hashmap_item_deinit(item);
            free(item);
        }
    }

    free(hm->table);

}


