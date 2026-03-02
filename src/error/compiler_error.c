
#include "compiler_error.h"

void compiler_error_deinit(struct compiler_error *err) {
    free(err->msg);
}

void print_compiler_error(FILE *file, struct compiler_error *err) {

    const char *kind_s;
    switch (err->kind) {
        case LEXER_ERROR:
        kind_s = "LEXER ERROR";
        break;
        case PARSER_ERROR:
        kind_s = "PARSER ERROR";
        break;
    }
    fprintf(file, "%s [%ld:%ld]: %s", kind_s, err->line, err->col, err->msg);
}
