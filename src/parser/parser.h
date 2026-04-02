
#ifndef __PARSER_HEADER__
#define __PARSER_HEADER__



#include "shared/ast.h"
#include "parser_impl.h"
#include "error/compiler_error.h"
#include <stdint.h>

#include "libs/vector/vector.h"
enum parser_result parse(struct token *in, uint32_t n, const char *filename, struct ast_file *out, struct compiler_error *err);



#endif
