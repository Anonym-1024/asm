
#ifndef __SEMA_HEADER__
#define __SEMA_HEADER__




#include "shared/ast.h"
#include "error/compiler_error.h"
#include "libs/hashmap/hashmap.h"



enum sema_result {
    SEMA_OK,
    SEMA_ERR
};







enum sema_result perform_semantic_analysis(struct ast_file *file, uint32_t *exec_start, uint32_t *data_start, struct compiler_error *error);

#endif
