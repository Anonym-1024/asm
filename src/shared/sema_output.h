#ifndef __SEMA_OUTPUT_HEADER__
#define __SEMA_OUTPUT_HEADER__



#include "libs/hashmap/hashmap.h"
#include <stdint.h>



struct sema_output {
    struct hashmap symbol_table;
    uint32_t code_len;
    uint32_t data_len;
};

void sema_output_deinit(struct sema_output *s);


#endif
