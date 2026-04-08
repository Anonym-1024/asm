
#include "codegen.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "error/compiler_error.h"
#include "libs/error_handling.h"
#include "libs/hashmap/hashmap.h"
#include "shared/ast.h"




struct codegen_context {
    struct sema_output *sema;
    FILE *out;
    uint32_t line;
    uint32_t col;
    char err_msg[ERR_MSG_LEN];
};



static enum codegen_result write_int32(FILE *out, uint32_t n) {
    uint8_t byte = n & 0xFF;
    for (int i = 0; i < 4; i++) {
        if (fwrite(&byte, 1, 1, out) != 1) {
            return CODEGEN_ERR;
        }
        n >>= 8;
        byte = n & 0xFF;
    }

    return CODEGEN_OK;
}



static enum codegen_result generate_byte_stmt(struct ast_byte_stmt *stmt, struct codegen_context *ctx) {
    if (stmt->init.kind == AST_INIT_NUM) {
        try_else(write_int32(ctx->out, stmt->init.number.token->number), CODEGEN_OK, return CODEGEN_ERR);
    } else {
        try_else(write_int32(ctx->out, 0), CODEGEN_OK, return CODEGEN_ERR);
    }
    return CODEGEN_OK;

}

static enum codegen_result generate_bytes_stmt(struct ast_bytes_stmt *stmt, struct codegen_context *ctx) {

    if (stmt->init.kind == AST_INIT_NUM) {
        try_else(write_int32(ctx->out, stmt->init.number.token->number), CODEGEN_OK, return CODEGEN_ERR);
        for (uint32_t i = 0; i < (uint32_t)stmt->len.token->number - 4; i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, ctx->out), 1, return CODEGEN_ERR);
        }
    } else if (stmt->init.kind == AST_INIT_BYTE_INIT) {
        for (uint32_t i = 0; i < stmt->init.byte_init.num_n; i++) {
            try_else(fwrite(&(uint8_t){stmt->init.byte_init.numbers[i].token->number}, 1, 1, ctx->out), 1, return CODEGEN_ERR);
        }
        for (uint32_t i = 0; i < stmt->len.token->number - stmt->init.byte_init.num_n; i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, ctx->out), 1, return CODEGEN_ERR);
        }
    } else if (stmt->init.kind == AST_INIT_ASCII) {
        unsigned long len = strlen(stmt->init.ascii.token->lexeme);
        try_else(fwrite(stmt->init.ascii.token->lexeme, sizeof(char), len, ctx->out), len, return CODEGEN_ERR);
        for (uint32_t i = 0; i < (stmt->len.token->number - (uint32_t)len); i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, ctx->out), 1, return CODEGEN_ERR);
        }

    } else {
        for (uint32_t i = 0; i < (uint32_t)stmt->len.token->number; i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, ctx->out), 1, return CODEGEN_ERR);
        }
    }


    return CODEGEN_OK;
}

static enum codegen_result generate_data_stmt(struct ast_data_stmt *stmt, struct codegen_context *ctx) {
    if (stmt->kind == AST_DATA_STMT_BYTE) {
        try_else(generate_byte_stmt(&stmt->byte_stmt, ctx), CODEGEN_OK, return CODEGEN_ERR);
    } else if (stmt->kind == AST_DATA_STMT_BYTES) {
        try_else(generate_bytes_stmt(&stmt->bytes_stmt, ctx), CODEGEN_OK, return CODEGEN_ERR);
    }


    return CODEGEN_OK;


}

static enum codegen_result generate_data_stmts(struct ast_data_section *sec, struct codegen_context *ctx) {
    uint32_t i;
    for (i = 0; i < sec->stmts_n; i++) {
        try_else(generate_data_stmt(&sec->data_stmts[i], ctx), CODEGEN_OK, return CODEGEN_ERR);
    }


    return CODEGEN_OK;
}

static enum codegen_result generate_data_sections(struct ast_file *file, struct codegen_context *ctx) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_DATA_SECTION) {
            try_else(generate_data_stmts(&file->sections[i].data_section, ctx), CODEGEN_OK, return CODEGEN_ERR);
        }
    }
    return CODEGEN_OK;
}






static enum codegen_result resolve_loc_label(struct ast_loc_label *label, uint32_t pos, struct ast_code_section *sec, uint32_t *offset) {
    if (label->dir.token->dir == DIR_F) {
        for (uint32_t i = pos; i < sec->stmts_n; i++) {
            if (sec->code_stmts[i].kind == AST_CODE_STMT_LOC_LABEL) {
                *offset = sec->code_stmts[i].loc_label_stmt.offset;
                return CODEGEN_OK;
            }
        }
    } else {
        for (int64_t i = pos; i >= 0; i--) {
            if (sec->code_stmts[i].kind == AST_CODE_STMT_LOC_LABEL) {
                *offset = sec->code_stmts[i].loc_label_stmt.offset;
                return CODEGEN_OK;
            }
        }
    }

    return CODEGEN_ERR;
}


static void add_arg(uint8_t *byte1, uint8_t *byte2, uint32_t argc, uint32_t arg) {

    switch (argc) {
        case 0:
            *byte1 |= arg;
            break;

        case 1:
            *byte2 |= arg << 4;
            break;

        case 2:
            *byte2 |= arg;
            break;
    }
}

static void add_imm16(uint8_t *byte2, uint8_t *byte3, uint16_t imm) {
    *byte2 |= imm;
    *byte3 |= (imm >> 8);
}




static enum codegen_result generate_instruction_stmt(struct ast_instruction_stmt *instr, struct codegen_context *ctx, struct ast_code_section *sec, uint32_t pos) {
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;
    uint8_t byte2 = 0;
    uint8_t byte3 = 0;

    uint32_t opcode = instr->instr.token->instr << 1;


    byte0 |= (opcode & (0xF0)) >> 4;
    byte1 |= (opcode & (0x0F)) << 4;

    if (instr->condition_code.token != NULL) {
        byte0 |= instr->condition_code.token->cond_code << 4;
    }


    const char *label = "";
    uint32_t offset;
    for (uint32_t i = 0; i < instr->args_n; i++) {
        struct ast_arg *arg = &instr->args[i];
        switch (instr->args[i].kind) {
            case AST_ARG_REG:
                add_arg(&byte1, &byte2, i, arg->reg.token->reg);
                break;
            case AST_ARG_PORT:
                add_arg(&byte1, &byte2, i, arg->reg.token->port);
                break;
            case AST_ARG_SYS_REG:
                add_arg(&byte1, &byte2, i, arg->reg.token->sys_reg);
                break;
            case AST_ARG_ADDR_REG:
                add_arg(&byte1, &byte2, i, arg->reg.token->addr_reg);
                break;
            case AST_ARG_LABEL:
                byte1 |= 0x10;
                label = arg->label.ident.token->lexeme;


                if (hashmap_get(&ctx->sema->symbol_table, label, &offset) == HMAP_OK) {
                    add_imm16(&byte2, &byte3, offset);
                    label = "~";
                } else if (hashmap_get(&ctx->sema->external_symbol_table, label, &offset) != HMAP_OK) {

                    snprintf(ctx->err_msg, ERR_MSG_LEN, "Label '%.20s' has not been declared.", label);
                    ctx->col = arg->label.ident.token->col;
                    return CODEGEN_ERR;
                }
                break;
            case AST_ARG_LOC_LABEL:
                byte1 |= 0x10;

                if(resolve_loc_label(&arg->loc_label, pos, sec, &offset) != CODEGEN_OK) {
                   snprintf(ctx->err_msg, ERR_MSG_LEN, "Could not resolve local label '%.20s'", arg->loc_label.ident.token->lexeme);
                   ctx->col = arg->loc_label.ident.token->col;
                   return CODEGEN_ERR;
                }
                add_imm16(&byte2, &byte3, offset);
                label = "~";
                break;
            case AST_ARG_IMMEDIATE:
                byte1 |= 0x10;
                byte3 |= arg->immediate.token->number;
                break;
        }
    }

    try_else(fwrite(&byte0, sizeof(uint8_t), 1, ctx->out), 1, return CODEGEN_ERR);
    try_else(fwrite(&byte1, sizeof(uint8_t), 1, ctx->out), 1, return CODEGEN_ERR);
    try_else(fwrite(&byte2, sizeof(uint8_t), 1, ctx->out), 1, return CODEGEN_ERR);
    try_else(fwrite(&byte3, sizeof(uint8_t), 1, ctx->out), 1, return CODEGEN_ERR);
    unsigned int label_len = strlen(label) + 1;
    try_else(fwrite(label, sizeof(char), label_len, ctx->out), label_len, return CODEGEN_ERR);

    return CODEGEN_OK;
}
static enum codegen_result generate_code_stmt(struct ast_code_stmt *stmt, struct codegen_context *ctx, struct ast_code_section *sec, uint32_t pos) {
    if (stmt->kind == AST_CODE_STMT_INSTRUCTION) {
        try_else(generate_instruction_stmt(&stmt->instruction_stmt, ctx, sec, pos), CODEGEN_OK, goto _error);
    }

    return CODEGEN_OK;

_error:
    ctx->line = stmt->line;
    return CODEGEN_ERR;

}

static enum codegen_result generate_code_stmts(struct ast_code_section *sec, struct codegen_context *ctx) {
    uint32_t i;
    for (i = 0; i < sec->stmts_n; i++) {
        try_else(generate_code_stmt(&sec->code_stmts[i], ctx, sec, i), CODEGEN_OK, return CODEGEN_ERR);
    }


    return CODEGEN_OK;
}

static enum codegen_result generate_code_sections(struct ast_file *file, struct codegen_context *ctx) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_CODE_SECTION) {
            try_else(generate_code_stmts(&file->sections[i].code_section, ctx), CODEGEN_OK, return CODEGEN_ERR);
        }
    }
    return CODEGEN_OK;
}


static enum codegen_result generate_symbol_table(struct hashmap *table, FILE *out) {
    for (size_t i = 0; i < table->size; i++) {
        struct hashmap_item *item = table->table[i];
        while (item != NULL) {

            unsigned long len = strlen(item->key) + 1;
            if (fwrite(item->key, sizeof(char), len, out) != len) {
                return CODEGEN_ERR;
            }
            try_else(write_int32(out, item->value), CODEGEN_OK, return CODEGEN_ERR);

            item = item->next;
        }
    }


    return CODEGEN_OK;
}

enum codegen_result generate_object_file(struct ast_file *file, struct sema_output *sema, const char *_out, struct compiler_error *err) {
    struct codegen_context ctx = {
        .sema = sema,
        .out = NULL,
        .line = 0,
        .col = 0,
    };
    strcpy(ctx.err_msg, "Unknown error.");



    FILE *out = fopen(_out, "wb");
    if (out == NULL) {
        goto _error;
    }
    ctx.out = out;

    uint32_t symbol_n = hashmap_get_item_count(&sema->global_symbol_table);

    try_else(write_int32(out, symbol_n), CODEGEN_OK, goto _error);
    try_else(write_int32(out, sema->code_len), CODEGEN_OK, goto _error);
    try_else(write_int32(out, sema->data_len), CODEGEN_OK, goto _error);



    try_else(generate_symbol_table(&sema->global_symbol_table, out), CODEGEN_OK, goto _error);

    try_else(generate_code_sections(file, &ctx), CODEGEN_OK, goto _error);

    try_else(generate_data_sections(file, &ctx), CODEGEN_OK, goto _error);


    fclose(out);

    return CODEGEN_OK;

_error:
    fclose(out);
    err->kind = CERROR_CODEGEN;
    err->file = file->filename;
    err->line = ctx.line;
    err->col = ctx.col;
    strcpy(err->msg, ctx.err_msg);

    return CODEGEN_ERR;
}
