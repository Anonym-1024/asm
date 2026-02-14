
#include "parser_error.h"

char *parser_error_desc(struct parser_error *err) {
    char *out = NULL;
    asprintf(&out, "PARSER ERROR [%ld:%ld]: %s", err->line, err->col, err->msg);
    return out;
}
