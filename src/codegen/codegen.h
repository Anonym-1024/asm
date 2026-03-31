
#ifndef __CODEGEN_HEADER__
#define __CODEGEN_HEADER__



#include <stdint.h>
#include "shared/ast.h"
#include "libs/error_handling.h"
#include "libs/vector/vector.h"
#include "shared/compiled_object.h"
#include "error/compiler_error.h"



enum codegen_result {
    CODEGEN_OK,
    CODEGEN_ERR
};


enum codegen_result generate_compiled_object(struct ast_file *file, FILE *obj, struct compiler_error *err);

#endif
