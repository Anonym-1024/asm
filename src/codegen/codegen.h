
#ifndef __CODEGEN_HEADER__
#define __CODEGEN_HEADER__



#include <stdint.h>
#include <stdio.h>
#include "shared/ast.h"
#include "shared/compiled_object.h"
#include "error/compiler_error.h"
#include "shared/sema_output.h"
#include <stdint.h>



enum codegen_result {
    CODEGEN_OK,
    CODEGEN_ERR
};


enum codegen_result generate_compiled_object(struct ast_file *file, struct compiled_object *obj, struct compiler_error *err);

#endif
