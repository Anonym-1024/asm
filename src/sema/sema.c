

#include "sema.h"

#include "libs/error_handling.h"
#include "error/compiler_error.h"
#include "libs/hashmap/hashmap.h"
#include "shared/ast.h"
#include "shared/sema_output.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>



/*
    1) scan data, control inits, calculate sizes, save label offsets
    2) scan instructions, control args, calculate label offsets
    3) return symbol table, exec len, data len, 
    4)
    5)
    6)



*/

enum sema_arg_kind {
    SEMA_ARG_REG = 1 << 0,
    SEMA_ARG_SYS_REG = 1 << 1,
    SEMA_ARG_ADDR_REG = 1 << 2,
    SEMA_ARG_PORT = 1 << 3,
    SEMA_ARG_IMM8 = 1 << 4,
    SEMA_ARG_IMM16 = 1 << 5,
    SEMA_ARG_DATA_LABEL = 1 << 6,
    SEMA_ARG_EXEC_LABEL = 1 << 7
};



struct sema_context {
    uint32_t line;
    uint16_t col;
    char error_msg[ERR_MSG_LEN];

    struct hashmap symbol_table;
    uint32_t exec_len;
    uint32_t data_len;


    enum sema_arg_kind instr_f[64][3];

};


void generate_instr_format(struct sema_context *ctx) {
    #define X(u, l, arg1, arg2, arg3) \
    ctx->instr_f[INSTR_##u][0] = arg1; \
    ctx->instr_f[INSTR_##u][1] = arg2; \
    ctx->instr_f[INSTR_##u][2] = arg3;
    #include "resources/instructions.def"
    #undef X
}






enum sema_result analyze_loc_label_stmt(struct ast_loc_label_stmt *stmt, struct sema_context *ctx) {
    stmt->offset = ctx->exec_len;
    return SEMA_OK;
}

enum sema_result analyze_exec_label_stmt(struct ast_label_stmt *stmt, struct sema_context *ctx) {
    if (hashmap_find(&ctx->symbol_table, stmt->ident.token->lexeme) == HMAP_OK) {
        ctx->col = stmt->ident.token->col;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Redefinition of label '%.20s'.", stmt->ident.token->lexeme);
        return SEMA_ERR;
    } 

    try_else(hashmap_add(&ctx->symbol_table, stmt->ident.token->lexeme, ctx->exec_len), HMAP_OK, return SEMA_ERR);

    return SEMA_OK;
}

enum sema_result analyze_start_stmt(struct sema_context *ctx) {
    if (hashmap_find(&ctx->symbol_table, ".start") == HMAP_OK) {
        ctx->col = 0;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Redefinition of '.start' label.");
        return SEMA_ERR;
    } 

    try_else(hashmap_add(&ctx->symbol_table, ".start", ctx->exec_len), HMAP_OK, return SEMA_ERR);

    return SEMA_OK;
}

enum sema_result analyze_data_label_stmt(struct ast_label_stmt *stmt, struct sema_context *ctx) {
    if (hashmap_find(&ctx->symbol_table, stmt->ident.token->lexeme) == HMAP_OK) {
        ctx->col = stmt->ident.token->col;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Redefinition of label '%.20s'.", stmt->ident.token->lexeme);
        return SEMA_ERR;
    } 

    try_else(hashmap_add(&ctx->symbol_table, stmt->ident.token->lexeme, ctx->data_len + ctx->exec_len), HMAP_OK, return SEMA_ERR);

    return SEMA_OK;
}


enum sema_result analyze_bytes_stmt(struct ast_bytes_stmt *stmt, struct sema_context *ctx) {
    int32_t len = stmt->len.token->number;
    if (len <= 0) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Length has to be at least 1.");
        ctx->col = stmt->len.token->col;
        return SEMA_ERR;
    }
    
    if (stmt->init.kind == AST_INIT_BYTE_INIT) {
        if (stmt->init.byte_init.num_c > (uint32_t)len) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Byte initializer is too large.");
            ctx->col = stmt->len.token->col;
            return SEMA_ERR;
        }

        for (uint32_t i = 0; i < stmt->init.byte_init.num_c; i++) {
            if (!(stmt->init.byte_init.numbers[i].token->number >= INT8_MIN && stmt->init.byte_init.numbers[i].token->number <= UINT8_MAX)) {
                snprintf(ctx->error_msg, ERR_MSG_LEN, "Byte initializer out of bounds.");
                ctx->col = stmt->init.byte_init.numbers[i].token->col;
                return SEMA_ERR;
            }
        }
    }

    if (stmt->init.kind == AST_INIT_ASCII) {
        if (strlen(stmt->init.ascii.token->lexeme) > (unsigned long)len) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Ascii literal is too large.");
            ctx->col = stmt->init.ascii.token->col;
            return SEMA_ERR;
        }
    }

    ctx->data_len += len;

    return SEMA_OK;

}

enum sema_result analyze_byte_stmt(struct ast_byte_stmt *stmt, struct sema_context *ctx) {
    if (stmt->init.kind == AST_INIT_BYTE_INIT || stmt->init.kind == AST_INIT_ASCII) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Byte can only be initialized with a number.");
        ctx->col = 0;
        return SEMA_ERR;
    }

    if (stmt->init.kind == AST_INIT_NUM && !(stmt->init.number.token->number >= INT8_MIN && stmt->init.number.token->number <= UINT8_MAX)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Byte initializer out of bounds.");
        ctx->col = stmt->init.number.token->col;
        return SEMA_ERR;
    }

    ctx->data_len += 1;

    return SEMA_OK;
}

enum sema_result analyze_data_stmt(struct ast_data_stmt *stmt, struct sema_context *ctx) {
    if (stmt->kind == AST_DATA_STMT_BYTE_STMT) {
        try_else(analyze_byte_stmt(&stmt->byte_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_DATA_STMT_BYTES_STMT) {
        try_else(analyze_bytes_stmt(&stmt->bytes_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_DATA_STMT_LABEL_STMT) {
        try_else(analyze_data_label_stmt(&stmt->label_stmt, ctx), SEMA_OK, goto _error);
    }
    

    return SEMA_OK;

_error:
    ctx->line = stmt->line;
    return SEMA_ERR;
}

enum sema_result analyze_data_stmts(struct ast_data_section *sec, struct sema_context *ctx) {
    uint32_t i;
    for (i = 0; i < sec->stmts_c; i++) {
        try_else(analyze_data_stmt(&sec->data_stmts[i], ctx), SEMA_OK, return SEMA_ERR);
    }
    if (sec->data_stmts[i].kind == AST_DATA_STMT_LABEL_STMT) {
        ctx->line = sec->data_stmts[i].line;
        ctx->col = sec->data_stmts[i].label_stmt.ident.token->col;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Label '%.20s' does not point to anything", sec->data_stmts[i].label_stmt.ident.token->lexeme);
        return SEMA_ERR;

    }

    return SEMA_OK;
}

enum sema_result analyze_data_sections(struct ast_file *file, struct sema_context *ctx) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_DATA_SECTION) {
            try_else(analyze_data_stmts(&file->sections[i].data_section, ctx), SEMA_OK, return SEMA_ERR);
        }
    }
    return SEMA_OK;
}






enum sema_result analyze_instruction_stmt(struct ast_instruction_stmt *stmt, struct sema_context *ctx) {
    enum instruction_token instr = stmt->instr.token->instr;
    for (uint32_t i = 0; i < 3; i++) {
        
        enum sema_arg_kind req = ctx->instr_f[instr][i];
        struct ast_arg *arg = &stmt->args[i];
        if (req == 0 && stmt->args_c > i) {
            ctx->col = stmt->instr.token->col;
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Too many arguments.");
            return SEMA_ERR;
        } else if (req == 0) {
            break;
        }

        if (i >= stmt->args_c) {
            ctx->col = stmt->instr.token->col;
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Too few arguments.");
            return SEMA_ERR;
        }

        if ((req & SEMA_ARG_REG) && arg->kind == AST_ARG_REG) {
            continue;
        } 

        if ((req & SEMA_ARG_SYS_REG) && arg->kind == AST_ARG_SYS_REG) {
            continue;
        } 

        if ((req & SEMA_ARG_ADDR_REG) && arg->kind == AST_ARG_ADDR_REG) {
           continue;
        } 

        if ((req & SEMA_ARG_PORT) && arg->kind == AST_ARG_PORT) {
            continue;
        } 

        if ((req & SEMA_ARG_IMM8) && arg->kind == AST_ARG_IMMEDIATE) {
            if (arg->immediate.token->number < INT8_MIN || arg->immediate.token->number > UINT8_MAX) {
                ctx->col = stmt->args[i].immediate.token->col;
                snprintf(ctx->error_msg, ERR_MSG_LEN, "Immediate out of bounds.");
                return SEMA_ERR;
            }
            continue;
        } 

        if ((req & SEMA_ARG_IMM16) && arg->kind == AST_ARG_IMMEDIATE) {
            if (arg->immediate.token->number < INT16_MIN || arg->immediate.token->number > UINT16_MAX) {
                ctx->col = stmt->args[i].immediate.token->col;
                snprintf(ctx->error_msg, ERR_MSG_LEN, "Immediate out of bounds.");
                return SEMA_ERR;
            }
            continue;
        } 

        if ((req & SEMA_ARG_DATA_LABEL) && arg->kind == AST_ARG_LABEL) {
            continue;
        } 

        if ((req & SEMA_ARG_EXEC_LABEL) && arg->kind == AST_ARG_LABEL) {
            continue;
        } 

        if ((req & SEMA_ARG_EXEC_LABEL) && arg->kind == AST_ARG_LOC_LABEL) {
            continue;
        } 

        ctx->col = stmt->instr.token->col;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Invalid argument at position %d.", i+1);
        return SEMA_ERR;
    }
    

    ctx->exec_len += 4;
    return SEMA_OK;


}

enum sema_result analyze_exec_stmt(struct ast_exec_stmt *stmt, struct sema_context *ctx) {
    if (stmt->kind == AST_EXEC_STMT_INSTRUCTION_STMT) {
        try_else(analyze_instruction_stmt(&stmt->instruction_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_EXEC_STMT_LABEL_STMT) {
        try_else(analyze_exec_label_stmt(&stmt->label_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
        try_else(analyze_loc_label_stmt(&stmt->loc_label_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_EXEC_STMT_START_STMT) {
        try_else(analyze_start_stmt(ctx), SEMA_OK, goto _error);
    }

    return SEMA_OK;
        
_error: 
    ctx->line = stmt->line;
    return SEMA_ERR;
}

enum sema_result analyze_exec_stmts(struct ast_exec_section *sec, struct sema_context *ctx) {
    uint32_t i;
    for (i = 0; i < sec->stmts_c; i++) {
        try_else(analyze_exec_stmt(&sec->exec_stmts[i], ctx), SEMA_OK, return SEMA_ERR);
    }

    if (sec->exec_stmts[i].kind == AST_EXEC_STMT_LABEL_STMT) {
        ctx->line = sec->exec_stmts[i].line;
        ctx->col = sec->exec_stmts[i].label_stmt.ident.token->col;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Label '%.20s' does not point to anything", sec->exec_stmts[i].label_stmt.ident.token->lexeme);
        return SEMA_ERR;
    }

    if (sec->exec_stmts[i].kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
        ctx->line = sec->exec_stmts[i].line;
        ctx->col = sec->exec_stmts[i].label_stmt.ident.token->col;
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Local label does not point to anything");
        return SEMA_ERR;

    }
    return SEMA_OK;
}

enum sema_result analyze_exec_sections(struct ast_file *file, struct sema_context *ctx) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_EXEC_SECTION) {
            try_else(analyze_exec_stmts(&file->sections[i].exec_section, ctx), SEMA_OK, return SEMA_ERR);
        }
    }
    return SEMA_OK;
}








enum sema_result perform_semantic_analysis(struct ast_file *file, struct sema_output *out, struct compiler_error *error) {
    struct sema_context ctx = {
        .col = 0,
        .line = 1,
        .data_len = 0,
        .exec_len = 0,
    };


    try_else(hashmap_init(&ctx.symbol_table, 256), HMAP_OK, goto _error);


    generate_instr_format(&ctx);

    try_else(analyze_exec_sections(file, &ctx), SEMA_OK, goto _error);

    try_else(analyze_data_sections(file, &ctx), SEMA_OK, goto _error);

    


    out->symbol_table = ctx.symbol_table;
    out->exec_len = ctx.exec_len;
    out->data_len = ctx.data_len;
    
    return SEMA_OK;

_error: 

    hashmap_deinit(&ctx.symbol_table);

    error->kind = CERROR_SEMANTIC;
    error->col = ctx.col;
    error->line = ctx.line;
    strcpy(error->msg, ctx.error_msg);
    return SEMA_ERR;
}

