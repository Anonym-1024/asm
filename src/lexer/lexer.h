
#ifndef __LEXER_HEADER__
#define __LEXER_HEADER__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>





#include "error/compiler_error.h"


#include "shared/token.h"
#include "shared/source_file.h"

enum lexer_result {
    LEX_OK,
    LEX_ERR
};


int get_token_desc(struct token *t, char **out);


enum lexer_result tokenise(struct source_file *in, struct token **out, uint32_t *out_n, struct compiler_error *error);

#endif
