
#ifndef __COMPILER_ERROR_HEADER__
#define __COMPILER_ERROR_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


enum compiler_error_kind {
    LEXER_ERROR,
    PARSER_ERROR,
};

struct compiler_error {
    enum compiler_error_kind kind;
    size_t line;
    size_t col;
    char *msg;
};

void compiler_error_deinit(struct compiler_error *err);

void print_compiler_error(FILE *file, struct compiler_error *err);

#endif
