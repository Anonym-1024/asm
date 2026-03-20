
#ifndef __COMPILER_ERROR_HEADER__
#define __COMPILER_ERROR_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


enum compiler_error_kind {
    CERROR_LEXER,
    CERROR_PARSER,
    CERROR_SEMANTIC
};

#define ERR_MSG_LEN 100

struct compiler_error {
    enum compiler_error_kind kind;
    size_t line;
    size_t col;
    char msg[ERR_MSG_LEN];
};



void print_compiler_error(FILE *file, struct compiler_error *err);

#endif
