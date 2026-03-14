
#ifndef __SEMA_HEADER__
#define __SEMA_HEADER__




#include "shared/ast.h"
#include "error/compiler_error.h"
#include "libs/hashmap/hashmap.h"



enum sema_result {
    SEMA_OK,
    SEMA_ERR
};


enum sema_result build_symbol_tables(struct ast_file *file);




enum sema_result perform_semantic_analysis(struct ast_file *file, struct compiler_error *error);

#endif
