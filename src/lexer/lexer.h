
#ifndef __LEXER_HEADER__
#define __LEXER_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "libs/utilities/utilities.h"

#include "resources/resources.h"
#include "lexer_error.h"

enum token_kind {
    TOKEN_DIR,
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
    unsigned long line;
};

struct lexer_error tokenise(const char *in, unsigned long n, struct token *out);

#endif