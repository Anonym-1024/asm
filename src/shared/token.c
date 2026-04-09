
#include "token.h"

#include <stdlib.h>

void token_deinit(struct token *ptr) {
    if (ptr->kind == TOKEN_IDENT || ptr->kind == TOKEN_ASCII) {
        free(ptr->lexeme);
    }
}

void _token_deinit(void *ptr) {
    token_deinit(ptr);
}
