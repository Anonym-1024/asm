
#include "parser.h"


enum parser_result parse(const struct token *in, size_t n, struct ast_file *out, struct compiler_error *err) {

    struct parser_context ctx = {
        .in = in,
        .n = n,
        .index = 0,
        
        .line = 1,
        .col = 1,
        .error_msg = NULL
    };

    enum parser_result res = parse_file(&ctx, out);

    if (res == PARSER_OK) {
        return PARSER_OK;
    }

    if (ctx.error_msg == NULL) {
        ctx.error_msg = "Unknown error.";
    }

    err->col = ctx.col;
    err->line = ctx.line;
    err->kind = PARSER_ERROR;
    err->msg = ctx.error_msg;

    return PARSER_ERR;
}
