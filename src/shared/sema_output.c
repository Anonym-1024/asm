
#include "sema_output.h"

void sema_output_deinit(struct sema_output *s) {
    hashmap_deinit(&s->symbol_table);
}