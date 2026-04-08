
#include "parser.h"


enum parser_result parse(struct token *in, uint32_t n, const char *filename, struct ast_file *out, struct compiler_error *err) {


    struct parser_context ctx = {
        .in = in,
        .n = n,
        .index = 0,

        .line = 1,
        .col = 1
    };
    strcpy(ctx.error_msg, "Unknown error");

    enum parser_result res = parse_file(&ctx, out);

    if (res == PARSER_OK) {
        out->filename = filename;
        return PARSER_OK;
    }



    err->col = ctx.col;
    err->line = ctx.line;
    err->kind = CERROR_PARSER;
    err->file = filename;
    strcpy(err->msg, ctx.error_msg);

    return PARSER_ERR;
}
