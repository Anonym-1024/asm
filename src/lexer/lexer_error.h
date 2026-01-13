
#ifndef __LEXER_ERROR_HEADER__
#define __LEXER_ERROR_HEADER__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


enum lexer_error_kind {
    LEX_UNKNOWN_ERR,
    LEX_MEM_ERR,
    LEX_INV_DIR,
    LEX_INV_MACRO,
    LEX_INV_CHAR,
    LEX_INV_ASCII_LIT,
    LEX_UNTERM_ASCII_LIT,
    LEX_INV_NUM
};

struct lexer_error {
    enum lexer_error_kind kind;
    size_t line;
    size_t col;
};

char *lexer_error_desc(struct lexer_error *err);

#endif
