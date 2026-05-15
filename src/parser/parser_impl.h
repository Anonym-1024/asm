
#ifndef __PARSER_IMPL_HEADER__
#define __PARSER_IMPL_HEADER__


#include "error/compiler_error.h"
#include "shared/token.h"
#include "shared/ast.h"
#include <stdint.h>

struct parser_context {
    struct token *in;
    uint32_t n;
    uint32_t index;

    uint32_t line;
    uint16_t col;
    char error_msg[ERR_MSG_LEN + 1];
};


enum parser_result {
    PARSER_OK,
    PARSER_ERR
};


#endif
