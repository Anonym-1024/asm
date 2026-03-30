

#include "sema.h"

#include "libs/error_handling.h"
#include "error/compiler_error.h"
#include "shared/ast.h"
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
    struct compiler_error err;

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




enum sema_result analyze_bytes_stmt(struct ast_bytes_stmt *stmt, struct sema_context *ctx) {
    int32_t len = stmt->len.token->number;
    if (len <= 0) {
        snprintf(ctx->err.msg, ERR_MSG_LEN, "Length has to be at least 1.");
        ctx->err.col = stmt->len.token->col;
        return SEMA_ERR;
    }
    
    if (stmt->init.kind == AST_INIT_BYTE_INIT) {
        if (stmt->init.byte_init.num_c > (uint32_t)len) {
            snprintf(ctx->err.msg, ERR_MSG_LEN, "Byte initializer is too large.");
            ctx->err.col = stmt->len.token->col;
            return SEMA_ERR;
        }

        for (uint32_t i = 0; i < stmt->init.byte_init.num_c; i++) {
            if (!(stmt->init.byte_init.numbers[i].token->number >= INT8_MIN && stmt->init.byte_init.numbers[i].token->number <= UINT8_MAX)) {
                snprintf(ctx->err.msg, ERR_MSG_LEN, "Byte initializer out of bounds.");
                ctx->err.col = stmt->init.byte_init.numbers[i].token->col;
                return SEMA_ERR;
            }
        }
    }

    if (stmt->init.kind == AST_INIT_ASCII) {
        if (strlen(stmt->init.ascii.token->lexeme) > (unsigned long)len) {
            snprintf(ctx->err.msg, ERR_MSG_LEN, "Ascii literal is too large.");
            ctx->err.col = stmt->init.ascii.token->col;
            return SEMA_ERR;
        }
    }

    return SEMA_OK;

}

enum sema_result analyze_byte_stmt(struct ast_byte_stmt *stmt, struct sema_context *ctx) {
    if (stmt->init.kind == AST_INIT_BYTE_INIT || stmt->init.kind == AST_INIT_ASCII) {
        snprintf(ctx->err.msg, ERR_MSG_LEN, "Byte can only be initialized with a number.");
        ctx->err.col = 0;
        return SEMA_ERR;
    }

    if (stmt->init.kind == AST_INIT_NUM && !(stmt->init.number.token->number >= INT8_MIN && stmt->init.number.token->number <= UINT8_MAX)) {
        snprintf(ctx->err.msg, ERR_MSG_LEN, "Byte initializer out of bounds.");
        ctx->err.col = stmt->init.number.token->col;
        return SEMA_ERR;
    }
    return SEMA_OK;
}

enum sema_result analyze_data_stmt(struct ast_data_stmt *stmt, struct sema_context *ctx) {
    if (stmt->kind == AST_DATA_STMT_BYTE_STMT) {
        try_else(analyze_byte_stmt(&stmt->byte_stmt, ctx), SEMA_OK, goto _error);
    } else if (stmt->kind == AST_DATA_STMT_BYTES_STMT) {
        try_else(analyze_bytes_stmt(&stmt->bytes_stmt, ctx), SEMA_OK, goto _error);
    }
    

    return SEMA_OK;

_error:
    ctx->err.line = stmt->line;
    return SEMA_ERR;
}

enum sema_result analyze_data_stmts(struct ast_data_section *sec, struct sema_context *ctx) {
    for (uint32_t i = 0; i < sec->stmts_c; i++) {
        try_else(analyze_data_stmt(&sec->data_stmts[i], ctx), SEMA_OK, return SEMA_ERR);
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
            ctx->err.col = stmt->instr.token->col;
            snprintf(ctx->err.msg, ERR_MSG_LEN, "Too many arguments.");
            return SEMA_ERR;
        } else if (req == 0) {
            break;
        }

        if (i >= stmt->args_c) {
            ctx->err.col = stmt->instr.token->col;
            snprintf(ctx->err.msg, ERR_MSG_LEN, "Too few arguments.");
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
                ctx->err.col = stmt->args[i].immediate.token->col;
                snprintf(ctx->err.msg, ERR_MSG_LEN, "Immediate out of bounds.");
                return SEMA_ERR;
            }
            continue;
        } 

        if ((req & SEMA_ARG_IMM16) && arg->kind == AST_ARG_IMMEDIATE) {
            if (arg->immediate.token->number < INT16_MIN || arg->immediate.token->number > UINT16_MAX) {
                ctx->err.col = stmt->args[i].immediate.token->col;
                snprintf(ctx->err.msg, ERR_MSG_LEN, "Immediate out of bounds.");
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

        ctx->err.col = stmt->instr.token->col;
        snprintf(ctx->err.msg, ERR_MSG_LEN, "Invalid argument at position %d.", i+1);
        return SEMA_ERR;
    }
    

    return SEMA_OK;


}

enum sema_result analyze_exec_stmt(struct ast_exec_stmt *stmt, struct sema_context *ctx) {
    if (stmt->kind == AST_EXEC_STMT_INSTRUCTION_STMT) {
        try_else(analyze_instruction_stmt(&stmt->instruction_stmt, ctx), SEMA_OK, goto _error);
    }

    return SEMA_OK;
        
_error: 
    ctx->err.line = stmt->line;
    return SEMA_ERR;
}

enum sema_result analyze_exec_stmts(struct ast_exec_section *sec, struct sema_context *ctx) {
    for (uint32_t i = 0; i < sec->stmts_c; i++) {
        try_else(analyze_exec_stmt(&sec->exec_stmts[i], ctx), SEMA_OK, return SEMA_ERR);
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








enum sema_result perform_semantic_analysis(struct ast_file *file, struct compiler_error *error) {
    struct sema_context ctx;

    try_else(analyze_data_sections(file, &ctx), SEMA_OK, goto _error);

    generate_instr_format(&ctx);

    try_else(analyze_exec_sections(file, &ctx), SEMA_OK, goto _error);

    return SEMA_OK;

_error: 
    ctx.err.kind = CERROR_SEMANTIC;
    *error = ctx.err;
    return SEMA_ERR;
}

