
#include "ast.h"



#include <stdlib.h>



void ast_file_deinit(struct ast_file *file) {
    free(file->stmts);
}
