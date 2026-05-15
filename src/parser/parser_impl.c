
#include "parser_impl.h"

#include "error/compiler_error.h"
#include "libs/error_handling.h"
#include "libs/vector/vector.h"
#include "shared/ast.h"
#include "shared/token.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>




static bool is_matching_kind(struct parser_context *ctx, enum token_kind kind) {
    if ((ctx->index) >= ctx->n) {
        return false;
    }

    return ctx->in[ctx->index].kind == kind;
}

static bool is_matching_directive(struct parser_context *ctx, enum directive_token dir) {
    if ((ctx->index) >= ctx->n) {
        return false;
    }
    struct token t = ctx->in[ctx->index];
    return t.kind == TOKEN_DIR && t.dir == dir;
}

static bool is_matching_data_unit(struct parser_context *ctx, enum data_unit_token data_unit) {
    if ((ctx->index) >= ctx->n) {
        return false;
    }
    struct token t = ctx->in[ctx->index];
    return t.kind == TOKEN_DATA_UNIT && t.data_unit == data_unit;
}

static bool is_matching_punctuation(struct parser_context *ctx, enum punctuation_token punct) {
    if ((ctx->index) >= ctx->n) {
        return false;
    }
    struct token t = ctx->in[ctx->index];
    return t.kind == TOKEN_PUNCT && t.punct == punct;
}


static void next(struct parser_context *ctx) {
    if ((ctx->index + 1) >= ctx->n) {
        return;
    }

    ctx->index++;
    ctx->col = ctx->in[ctx->index].col;

}


static void pop_blank_lines(struct parser_context *ctx) {
    while (is_matching_punctuation(ctx, 0, PUNCT_NEWLINE)) {
        ctx->line++;
        next(ctx);
    }
}





enum parser_result parse_file(struct parser_context *ctx, struct ast_file *file) {

    pop_blank_lines(ctx);



    pop_blank_lines(ctx);

    if (!is_matching_kind(ctx, TOKEN_EOF)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected EOF");
        return PARSER_ERR;
    }

    return PARSER_OK;
}



enum parser_result parse_stmt(struct parser_context *ctx, struct ast_stmt *stmt) {
    if (is_matching_directive(ctx, DIR_CODE)) {

    } else if (is_matching_directive(ctx, DIR_CODE)) {

    } else if (is_matching_directive(ctx, DIR_)) {

    } else if () {

    } else if () {

    } else if () {

    } else if () {

    } else if () {

    } else if () {

    }
}
