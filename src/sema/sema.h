
#ifndef __SEMA_HEADER__
#define __SEMA_HEADER__




#include "shared/ast.h"
#include "error/compiler_error.h"
#include "libs/hashmap/hashmap.h"
#include <stdint.h>



enum sema_result {
    SEMA_OK,
    SEMA_ERR
};







enum sema_result perform_semantic_analysis(struct ast_file *file, uint32_t *start_addr, struct compiler_error *error);

#endif
