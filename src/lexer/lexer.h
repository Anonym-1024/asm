
#ifndef __LEXER_HEADER__
#define __LEXER_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



#include "lexer_error.h"
#include "libs/vector/vector.h"

enum token_kind {
    TOKEN_DIR,
    TOKEN_DATA_UNIT,
    TOKEN_INSTR,
    TOKEN_MACRO,
    TOKEN_COND_CODE,
    TOKEN_REG,
    TOKEN_SYS_REG,
    TOKEN_PORT,
    TOKEN_ADDR_REG,
    TOKEN_NUM,
    TOKEN_ASCII,
    TOKEN_IDENT,
    TOKEN_PUNCT,
    TOKEN_EOF
};

struct token {
    enum token_kind kind;
    char *lexeme;
    size_t line;
    size_t col;
};

enum lexer_result {
    LEX_OK,
    LEX_ERR
};


void token_desc(struct token *t, char **out);


enum lexer_result tokenise(const char *in, size_t n, struct vector *out, struct lexer_error *error);

#endif
