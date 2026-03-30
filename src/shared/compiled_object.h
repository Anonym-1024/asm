#ifndef __COMPILED_OBJECT_HEADER__
#define __COMPILED_OBJECT_HEADER__


#include "libs/hashmap/hashmap.h"
#include <stdint.h>



struct compiled_instruction {
    char *label_ref;
    uint8_t byte_0;
    uint8_t byte_1;
    uint8_t byte_2;
    uint8_t byte_3;
};

struct compiled_object {
    struct hashmap symbol_table;
    uint32_t exec_len;
    struct compiled_instruction *exec;
    uint32_t data_len;
    uint8_t *data;
};


#endif
