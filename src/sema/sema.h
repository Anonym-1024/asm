
#ifndef __SEMA_HEADER__
#define __SEMA_HEADER__




#include "parser/ast.h"
#include "libs/hashmap/hashmap.h"
#include "error/compiler_error.h"


struct sema_context {
    struct hashmap data_labels;
    struct hashmap exec_labels;
    struct hashmap local_labels;


    uint32_t data_offset;

    struct compiler_error err;


};

enum sema_result {
    SEMA_OK,
    SEMA_ERR
};


enum sema_result build_symbol_tables(struct ast_file *file, struct sema_context *ctx);

#endif
