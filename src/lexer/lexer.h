
#ifndef __LEXER_HEADER__
#define __LEXER_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "libs/utilities/utilities.h"



#include "error/compiler_error.h"
#include "libs/vector/vector.h"

#include "token.h"

enum lexer_result {
    LEX_OK,
    LEX_ERR
};


int get_token_desc(struct token *t, char **out);


enum lexer_result tokenise(const char *in, size_t n, struct token **out, size_t *out_n, struct compiler_error *error);

#endif
