

#include "sema.h"

#include "libs/error_handling.h"
#include "libs/hashmap/hashmap.h"
#include "error/compiler_error.h"
#include <errno.h>




enum sema_arg_type {
    SEMA_ARG_REG = 2 << 0,
    SEMA_ARG_SYS_REG = 2 << 1,
    SEMA_ARG_ADDR_REG = 2 << 2,
    SEMA_ARG_PORT = 2 << 3,
    SEMA_ARG_IMM8 = 2 << 4,
    SEMA_ARG_IMM16 = 2 << 5,
    SEMA_ARG_LABEL = 2 << 6,
    SEMA_ARG_LOC_LABEL = 2 << 7,
};


struct sema_context {
    
    struct hashmap labels;
    struct compiler_error error;
    uint32_t data_offset;
    uint32_t exec_offset;

    enum sema_arg_type instr_f[64][3];
};


enum sema_result analyze_data_bytes_stmt(struct ast_bytes_stmt *stmt, struct sema_context *ctx) {

    if (stmt->len.token->number < 1) {
        ctx->error.col = stmt->len.token->col;
        snprintf(ctx->error.msg, ERR_MSG_LEN, "Size has to be at least 1");
        return SEMA_ERR;
    }

    if (stmt->init.kind == AST_INIT_BYTE_INIT) {
        
        if (stmt->init.byte_init.num_c > (uint32_t)stmt->len.token->number) {
            ctx->error.col = stmt->init.byte_init.numbers[0].token->col;
            snprintf(ctx->error.msg, ERR_MSG_LEN, "Too many elements in byte list initializer.");
            return SEMA_ERR;
        }
        for (uint32_t n = 0; n < stmt->init.byte_init.num_c; n++) {
            if (stmt->init.byte_init.numbers[n].token->number < INT8_MIN || stmt->init.byte_init.numbers[n].token->number > UINT8_MAX) {
                ctx->error.col = stmt->init.byte_init.numbers[n].token->col;
                snprintf(ctx->error.msg, ERR_MSG_LEN, "Number literal is out of bounds.");
                return SEMA_ERR;
            }
        }
        
    }
    if (stmt->init.kind == AST_INIT_ASCII) {
        if (strlen(stmt->init.ascii.token->lexeme) > (unsigned long)stmt->len.token->number) {
            ctx->error.col = stmt->init.ascii.token->col;
            snprintf(ctx->error.msg, ERR_MSG_LEN, "Ascii literal is too large.");
            return SEMA_ERR;
        }
    }
    ctx->data_offset += stmt->len.token->number;
    return SEMA_OK;
}

enum sema_result analyze_data_byte_stmt(struct ast_byte_stmt *stmt, struct sema_context *ctx) {


    

    if (stmt->init.kind == AST_INIT_BYTE_INIT) {
        ctx->error.col = 0;
        snprintf(ctx->error.msg, ERR_MSG_LEN, "Byte cannot be initialized with a byte list.");
        return SEMA_ERR;
    }
    if (stmt->init.kind == AST_INIT_ASCII) {
        if (strlen(stmt->init.ascii.token->lexeme) > 1) {
            ctx->error.col = stmt->init.ascii.token->col;
            snprintf(ctx->error.msg, ERR_MSG_LEN, "Ascii literal is too large to fit in a byte.");
            return SEMA_ERR;
        }
    }
    if (stmt->init.kind == AST_INIT_NUM) {
        if (stmt->init.number.token->number < INT8_MIN || stmt->init.number.token->number > UINT8_MAX) {
            ctx->error.col = stmt->init.number.token->col;
            snprintf(ctx->error.msg, ERR_MSG_LEN, "Number literal is out of bounds.");
            return SEMA_ERR;
        }
    }
    ctx->data_offset += 1;
    return SEMA_OK;
}

enum sema_result analyze_data_label_stmt(struct ast_label_stmt *stmt, struct sema_context *ctx) {
    if (hashmap_find(&ctx->labels, stmt->ident.token->lexeme) == HMAP_OK) {
        ctx->error.col = stmt->ident.token->col;
        snprintf(ctx->error.msg, ERR_MSG_LEN, "Redefinition of label '.20%s'.", stmt->ident.token->lexeme);
        return SEMA_ERR;
    }

    try_else(hashmap_add(&ctx->labels, stmt->ident.token->lexeme, ctx->data_offset), HMAP_OK, return SEMA_ERR);
    return SEMA_OK;
}

enum sema_result analyze_data_stmt(struct ast_data_stmt *stmt, struct sema_context *ctx) {
    if (stmt->kind == AST_DATA_STMT_LABEL_STMT) {
        try_else(analyze_data_label_stmt(&stmt->label_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_DATA_STMT_BYTE_STMT) {
        try_else(analyze_data_byte_stmt(&stmt->byte_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_DATA_STMT_BYTES_STMT) {
        try_else(analyze_data_bytes_stmt(&stmt->bytes_stmt, ctx), SEMA_OK, goto _error);
    }
    return SEMA_OK;

_error:
    ctx->error.line = stmt->line;
    return SEMA_ERR;
}



enum sema_result analyze_data_section(struct ast_data_section *sec, struct sema_context *ctx) {
    for (uint32_t s = 0; s < sec->stmts_c; s++) {
        struct ast_data_stmt *stmt = &sec->data_stmts[s];
        try_else(analyze_data_stmt(stmt, ctx), SEMA_OK, return SEMA_ERR);
    }
    return SEMA_OK;
}

enum sema_result analyze_data_sections(struct ast_file *file, struct sema_context *ctx) {
    

    for (uint32_t s = 0; s < file->sec_n; s++) {
        struct ast_section *section = &file->sections[s];
        if (section->kind == AST_DATA_SECTION) {
            try_else(analyze_data_section(&section->data_section, ctx), SEMA_OK, return SEMA_ERR);
        }
    }
    return SEMA_OK;

}
















enum sema_result analyze_instruction_stmt(struct ast_instruction_stmt *stmt, struct sema_context *ctx) {
    enum instruction_token instr = stmt->instr.token->instr;
    for (uint32_t i = 0; i < 3; i++) {
        
        enum sema_arg_type req = ctx->instr_f[instr][i];
        struct ast_arg *arg = &stmt->args[i];
        if (req == 0 && stmt->args_c > i) {
            ctx->error.col = stmt->instr.token->col;
            snprintf(ctx->error.msg, ERR_MSG_LEN, "Too many arguments.");
            return SEMA_ERR;
        } else if (req == 0) {
            return SEMA_OK;
        }

        if (i >= stmt->args_c) {
            ctx->error.col = stmt->instr.token->col;
            snprintf(ctx->error.msg, ERR_MSG_LEN, "Too few arguments.");
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
                ctx->error.col = stmt->instr.token->col;
                snprintf(ctx->error.msg, ERR_MSG_LEN, "Immediate out of bounds.");
                return SEMA_ERR;
            }
            continue;
        } 

        if ((req & SEMA_ARG_IMM16) && arg->kind == AST_ARG_IMMEDIATE) {
            if (arg->immediate.token->number < INT16_MIN || arg->immediate.token->number > UINT16_MAX) {
                ctx->error.col = stmt->instr.token->col;
                snprintf(ctx->error.msg, ERR_MSG_LEN, "Immediate out of bounds.");
                return SEMA_ERR;
            }
            continue;
        } 

        if ((req & SEMA_ARG_LABEL) && arg->kind == AST_ARG_LABEL) {
            continue;
        } 

        if ((req & SEMA_ARG_LOC_LABEL) && arg->kind == AST_ARG_LOC_LABEL) {
            continue;
        } 

        ctx->error.col = stmt->instr.token->col;
        snprintf(ctx->error.msg, ERR_MSG_LEN, "Invalid argument at position %d.", i+1);
        return SEMA_ERR;
    }
    return SEMA_OK;
}




enum sema_result analyze_exec_label_stmt(struct ast_label_stmt *stmt, struct sema_context *ctx) {
    if (hashmap_find(&ctx->labels, stmt->ident.token->lexeme) == HMAP_OK) {
        ctx->error.col = stmt->ident.token->col;
        snprintf(ctx->error.msg, ERR_MSG_LEN, "Redefinition of label '%.20s'.", stmt->ident.token->lexeme);
        return SEMA_ERR;
    }

    try_else(hashmap_add(&ctx->labels, stmt->ident.token->lexeme, ctx->exec_offset), HMAP_OK, return SEMA_ERR);
    return SEMA_OK;
}


enum sema_result analyze_exec_start_stmt(struct sema_context *ctx) {
    if (hashmap_find(&ctx->labels, ".start") == HMAP_OK) {
        ctx->error.col = 0;
        snprintf(ctx->error.msg, ERR_MSG_LEN, "Redefinition of '.start' label.");
        return SEMA_ERR;
    }

    try_else(hashmap_add(&ctx->labels, ".start", ctx->exec_offset), HMAP_OK, return SEMA_ERR);
    return SEMA_OK;
}


enum sema_result analyze_exec_loc_label_stmt(struct ast_loc_label_stmt *stmt, struct sema_context *ctx) {
    stmt->offset = ctx->exec_offset;
    return SEMA_OK;
}


enum sema_result analyze_exec_stmt(struct ast_exec_stmt *stmt, struct sema_context *ctx) {
    
    if (stmt->kind == AST_EXEC_STMT_LABEL_STMT) {
        try_else(analyze_exec_label_stmt(&stmt->label_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_EXEC_STMT_START_STMT) {
        try_else(analyze_exec_start_stmt(ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_EXEC_STMT_INSTRUCTION_STMT) {
        try_else(analyze_instruction_stmt(&stmt->instruction_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
        try_else(analyze_exec_loc_label_stmt(&stmt->loc_label_stmt, ctx), SEMA_OK, goto _error);
    }


    return SEMA_OK;

_error:
    ctx->error.line = stmt->line;
    return SEMA_ERR;
}



enum sema_result analyze_exec_section(struct ast_exec_section *sec, struct sema_context *ctx) {
    for (uint32_t s = 0; s < sec->stmts_c; s++) {
        struct ast_exec_stmt *stmt = &sec->exec_stmts[s];
        try_else(analyze_exec_stmt(stmt, ctx), SEMA_OK, return SEMA_ERR);
    }
    return SEMA_OK;
}

enum sema_result find_local_label_offset(struct ast_exec_section *sec, uint32_t pos, struct ast_loc_label *l, struct sema_context *ctx) {
    if (l->dir.token->dir == DIR_F) {
        for (uint32_t i = pos; i < sec->stmts_c; i++) {
            if (sec->exec_stmts[i].kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
                l->offset = sec->exec_stmts[i].loc_label_stmt.offset;
                return SEMA_OK;
            }
        }
    } else {
        for (int64_t i = pos; i >= 0; i--) {
            if (sec->exec_stmts[i].kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
                l->offset = sec->exec_stmts[i].loc_label_stmt.offset;
                return SEMA_OK;
            }
        }
    }
    ctx->error.col = l->dir.token->col;
    snprintf(ctx->error.msg, ERR_MSG_LEN, "Could not find any local label in specified direction.");
    return SEMA_ERR;
}

enum sema_result expand_exec_stmt_labels(struct ast_exec_section *sec, uint32_t pos, struct ast_exec_stmt *stmt, struct sema_context *ctx) {
    if (stmt->kind != AST_EXEC_STMT_INSTRUCTION_STMT) {
        return SEMA_OK;
    }

    struct ast_instruction_stmt *instr = &stmt->instruction_stmt;

    uint32_t offset;
    for (uint32_t i = 0; i < instr->args_c; i++) {
        struct ast_arg *arg = &instr->args[i];
        if (instr->args[i].kind == AST_ARG_LABEL) {
            if (hashmap_get(&ctx->labels, arg->label.ident.token->lexeme, &offset) == HMAP_NO_ENTRY) {
                ctx->error.col = arg->label.ident.token->col;
                snprintf(ctx->error.msg, ERR_MSG_LEN, "Use of undeclared label '%.20s'", arg->label.ident.token->lexeme);
                goto _error;
            }
            arg->label.offset = offset;
        } else if (instr->args[i].kind == AST_ARG_LOC_LABEL) {
            try_else(find_local_label_offset(sec, pos, &arg->loc_label, ctx), SEMA_OK, goto _error);
        }
    }

    return SEMA_OK;
_error:
    ctx->error.line = stmt->line;
    return SEMA_ERR;
}


enum sema_result expand_exec_section_labels(struct ast_exec_section *sec, struct sema_context *ctx) {
    for (uint32_t s = 0; s < sec->stmts_c; s++) {
        struct ast_exec_stmt *stmt = &sec->exec_stmts[s];
        try_else(expand_exec_stmt_labels(sec, s, stmt, ctx), SEMA_OK, return SEMA_ERR);
    }
    return SEMA_OK;
}

enum sema_result analyze_exec_sections(struct ast_file *file, struct sema_context *ctx) {
    

    for (uint32_t s = 0; s < file->sec_n; s++) {
        struct ast_section *section = &file->sections[s];
        if (section->kind == AST_EXEC_SECTION) {
            try_else(analyze_exec_section(&section->exec_section, ctx), SEMA_OK, return SEMA_ERR);
        }
    }

    for (uint32_t s = 0; s < file->sec_n; s++) {
        struct ast_section *section = &file->sections[s];
        if (section->kind == AST_EXEC_SECTION) {
            try_else(expand_exec_section_labels(&section->exec_section, ctx), SEMA_OK, return SEMA_ERR);
        }
    }

    return SEMA_OK;

}


void init_arg_types(enum sema_arg_type f[64][3]) {
    #define X(u, l, a1, a2, a3) \
    f[INSTR_##u][0] = a1; \
    f[INSTR_##u][1] = a2; \
    f[INSTR_##u][2] = a3;
    #include "resources/instructions.def"
    #undef X
}














enum sema_result perform_semantic_analysis(struct ast_file *file, struct compiler_error *error) {
    struct sema_context ctx = {
        .data_offset = 0,
        .exec_offset = 0,
        .error.kind = CERROR_SEMANTIC
    };
    strncpy(ctx.error.msg, "Unknown error.", ERR_MSG_LEN);
    
    try_else(hashmap_init(&ctx.labels, 256), HMAP_OK, goto _error);
    
    
    init_arg_types(ctx.instr_f);

    try_else(analyze_data_sections(file, &ctx), SEMA_OK, goto _error);
    try_else(analyze_exec_sections(file, &ctx), SEMA_OK, goto _error);

    return SEMA_OK;

_error:
    *error = ctx.error;
    hashmap_deinit(&ctx.labels);
    
    return SEMA_ERR;
}
