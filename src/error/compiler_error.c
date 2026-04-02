
#include "compiler_error.h"



void print_compiler_error(FILE *file, struct compiler_error *err) {

    const char *kind_s;
    switch (err->kind) {
        case CERROR_LEXER:
        kind_s = "LEXER ERROR";
        break;
        case CERROR_PARSER:
        kind_s = "PARSER ERROR";
        break;

        case CERROR_SEMANTIC:
        kind_s = "SEMANTIC ERROR";
        break;

        case CERROR_CODEGEN:
        kind_s = "CODEGEN ERROR";
        break;

        case CERROR_LINKER:
        kind_s = "LINKER ERROR";
        break;

        default:
        kind_s = "";
        break;
    }
    fprintf(file, "\033[31m*** %s [%s:%ld:%ld]: %s\n", kind_s, err->file, err->line, err->col, err->msg);
}
