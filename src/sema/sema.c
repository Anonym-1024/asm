
#include "sema.h"

#include "libs/error_handling.h"

#include <errno.h>



static enum sema_result validate_number(struct token *t, int32_t *n) {
    unsigned long len = strlen(t->lexeme);

    char r = t->lexeme[len-2];
    
    int radix = 10;
    if (r == 'd') {
        radix = 10;
        t->lexeme[len-2] = 0;
    } else if (r == 'b') {
        radix = 2;
        t->lexeme[len-2] = 0;
    } else if (r == 'x') {
        radix = 16;
        t->lexeme[len-2] = 0;
    } 
    errno = 0;
    *n = strtol(t->lexeme, NULL, radix);
    if (errno != 0) {
        return SEMA_ERR;
    }

    return SEMA_OK;

}


static enum sema_result scan_data_label(struct ast_label_stmt *l, struct sema_context *ctx) {
    printf("label: %s", l->ident.token->lexeme);
    struct token *t = l->ident.token;
    if (hashmap_find(&ctx->data_labels, t->lexeme) == HMAP_OK) {
        ctx->err.line = t->line;
        ctx->err.col = t->col;
        snprintf(ctx->err.msg, ERR_MSG_LEN, "Redefinition of label '%.15s'.", t->lexeme);
        return SEMA_ERR;
    }
    try_else(hashmap_add(&ctx->data_labels, t->lexeme, ctx->data_offset), HMAP_OK, return SEMA_ERR);

    return SEMA_OK;
}

static enum sema_result scan_byte_stmt(struct ast_byte_stmt *s, struct sema_context *ctx) {
    ctx->data_offset += 1;

    if (s->is_initialized) {

    }
}

static enum sema_result scan_bytes_stmt(struct ast_bytes_stmt *s, struct sema_context *ctx) {
    
}

static enum sema_result scan_data_section(struct ast_data_section *sec, struct sema_context *ctx) {
    for (uint32_t i = 0; i < sec->stmts_c; i++) {
        struct ast_data_stmt *stmt = &sec->data_stmts[i];
        if (stmt->kind == AST_DATA_STMT_LABEL_STMT) {
            try_else(scan_data_label(&stmt->label_stmt, ctx), SEMA_OK, return SEMA_ERR);
        }
    }
    return SEMA_OK;
};

enum sema_result build_symbol_tables(struct ast_file *file, struct sema_context *ctx) {
    
    
    strcpy(ctx->err.msg, "Unknown error.");
    ctx->err.line = 0;
    ctx->err.col = 0;
    ctx->err.kind = CERROR_SEMANTIC;
    
    bool _data_l = false;
    bool _exec_l = false;
    bool _local_l = false;

    try_else(hashmap_init(&ctx->data_labels, 256), HMAP_OK, goto _error);
    _data_l = true;
    try_else(hashmap_init(&ctx->exec_labels, 256), HMAP_OK, goto _error);
    _exec_l = true;
    try_else(hashmap_init(&ctx->local_labels, 256), HMAP_OK, goto _error);
    _local_l = true;

    
    ctx->data_offset = 0;

    for (uint32_t s = 0; s < file->sec_n; s++) {
        printf(" %d", file->sections[s].kind);
        if (file->sections[s].kind == AST_DATA_SECTION) {
            printf(" a");
            try_else(scan_data_section(&file->sections[s].data_section, ctx), SEMA_OK, goto _error);
        } else {

        }
    }
    
    return SEMA_OK;

_error:
    if (_data_l) hashmap_deinit(&ctx->data_labels);
    if (_exec_l) hashmap_deinit(&ctx->exec_labels);
    if (_local_l) hashmap_deinit(&ctx->local_labels);

    
    return SEMA_ERR;
}
