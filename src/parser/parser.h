
#ifndef __PARSER_HEADER__
#define __PARSER_HEADER__



#include "ast.h"
#include "parser_impl.h"


enum parser_result parse(const struct token *in, size_t n, struct ast_file *out, struct compiler_error *err);

#endif
