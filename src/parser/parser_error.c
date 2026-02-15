
#include "parser_error.h"

int parser_error_desc(struct parser_error *err, char **desc) {
    char *out = NULL;
    if (asprintf(&out, "PARSER ERROR [%ld:%ld]: %s", err->line, err->col, err->msg) == -1) {
        free(out);
        return -1;
    }
    *desc = out;
    return 0;
}
