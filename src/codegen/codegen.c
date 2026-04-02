 
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





enum codegen_result write_int32(FILE *out, uint32_t n) {
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



enum codegen_result generate_byte_stmt(struct ast_byte_stmt *stmt, FILE *out) {
    if (stmt->init.kind == AST_INIT_NUM) {
        try_else(write_int32(out, stmt->init.number.token->number), CODEGEN_OK, return CODEGEN_ERR);
    } else {
        try_else(write_int32(out, 0), CODEGEN_OK, return CODEGEN_ERR);
    }
    return CODEGEN_OK;

}

enum codegen_result generate_bytes_stmt(struct ast_bytes_stmt *stmt, FILE *out) {
    
    if (stmt->init.kind == AST_INIT_NUM) {
        try_else(write_int32(out, stmt->init.number.token->number), CODEGEN_OK, return CODEGEN_ERR);
        for (uint32_t i = 0; i < (uint32_t)stmt->len.token->number - 4; i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, out), 1, return CODEGEN_ERR);
        }
    } else if (stmt->init.kind == AST_INIT_BYTE_INIT) {
        for (uint32_t i = 0; i < stmt->init.byte_init.num_c; i++) {
            try_else(fwrite(&(uint8_t){stmt->init.byte_init.numbers[i].token->number}, 1, 1, out), 1, return CODEGEN_ERR);
        }
        for (uint32_t i = 0; i < stmt->len.token->number - stmt->init.byte_init.num_c; i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, out), 1, return CODEGEN_ERR);
        }
    } else if (stmt->init.kind == AST_INIT_ASCII) {
        unsigned long len = strlen(stmt->init.ascii.token->lexeme);
        try_else(fwrite(stmt->init.ascii.token->lexeme, sizeof(char), len, out), len, return CODEGEN_ERR);
        for (uint32_t i = 0; i < (stmt->len.token->number - (uint32_t)len); i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, out), 1, return CODEGEN_ERR);
        }
        
    } else {
        for (uint32_t i = 0; i < (uint32_t)stmt->len.token->number; i++) {
            try_else(fwrite(&(uint8_t){0}, 1, 1, out), 1, return CODEGEN_ERR);
        }
    }
    

    return CODEGEN_OK;
}

enum codegen_result generate_data_stmt(struct ast_data_stmt *stmt, FILE *out) {
    if (stmt->kind == AST_DATA_STMT_BYTE_STMT) {
        try_else(generate_byte_stmt(&stmt->byte_stmt, out), CODEGEN_OK, return CODEGEN_ERR);
    } else if (stmt->kind == AST_DATA_STMT_BYTES_STMT) {
        try_else(generate_bytes_stmt(&stmt->bytes_stmt, out), CODEGEN_OK, return CODEGEN_ERR);
    }
    

    return CODEGEN_OK;


}

enum codegen_result generate_data_stmts(struct ast_data_section *sec, FILE *out) {
    uint32_t i;
    for (i = 0; i < sec->stmts_c; i++) {
        try_else(generate_data_stmt(&sec->data_stmts[i], out), CODEGEN_OK, return CODEGEN_ERR);
    }
    

    return CODEGEN_OK;
}

enum codegen_result generate_data_sections(struct ast_file *file, FILE *out) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_DATA_SECTION) {
            try_else(generate_data_stmts(&file->sections[i].data_section, out), CODEGEN_OK, return CODEGEN_ERR);
        }
    }
    return CODEGEN_OK;
}






enum codegen_result resolve_loc_label(struct ast_loc_label *label, uint32_t pos, struct ast_exec_section *sec, uint32_t *offset) {
    if (label->dir.token->dir == DIR_F) {
        for (uint32_t i = pos; i < sec->stmts_c; i++) {
            if (sec->exec_stmts[i].kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
                *offset = sec->exec_stmts[i].loc_label_stmt.offset;
                return CODEGEN_OK;
            }
        }
    } else {
        for (int64_t i = pos; i >= 0; i--) {
            if (sec->exec_stmts[i].kind == AST_EXEC_STMT_LOC_LABEL_STMT) {
                *offset = sec->exec_stmts[i].loc_label_stmt.offset;
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




enum codegen_result generate_instruction_stmt(struct ast_instruction_stmt *instr, FILE *out, struct compiler_error *err, struct ast_exec_section *sec, uint32_t pos) {
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
    for (uint32_t i = 0; i < instr->args_c; i++) {
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
                break;
            case AST_ARG_LOC_LABEL:
                byte1 |= 0x10;
                uint32_t offset;
                if(resolve_loc_label(&arg->loc_label, pos, sec, &offset) != CODEGEN_OK) {
                   snprintf(err->msg, ERR_MSG_LEN, "Could not resolve local label '%.20s'", arg->loc_label.ident.token->lexeme);
                   err->col = arg->loc_label.ident.token->col;
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

    try_else(fwrite(&byte0, sizeof(uint8_t), 1, out), 1, return CODEGEN_ERR);
    try_else(fwrite(&byte1, sizeof(uint8_t), 1, out), 1, return CODEGEN_ERR);
    try_else(fwrite(&byte2, sizeof(uint8_t), 1, out), 1, return CODEGEN_ERR);
    try_else(fwrite(&byte3, sizeof(uint8_t), 1, out), 1, return CODEGEN_ERR);
    unsigned int label_len = strlen(label) + 1;
    try_else(fwrite(label, sizeof(char), label_len, out), label_len, return CODEGEN_ERR);

    return CODEGEN_OK;
}
enum codegen_result generate_exec_stmt(struct ast_exec_stmt *stmt, FILE *out, struct compiler_error *err, struct ast_exec_section *sec, uint32_t pos) {
    if (stmt->kind == AST_EXEC_STMT_INSTRUCTION_STMT) {
        try_else(generate_instruction_stmt(&stmt->instruction_stmt, out, err, sec, pos), CODEGEN_OK, goto _error);
    }

    return CODEGEN_OK;
      
_error:
    err->line = stmt->line;
    return CODEGEN_ERR;

}

enum codegen_result generate_exec_stmts(struct ast_exec_section *sec, FILE *out, struct compiler_error *err) {
    uint32_t i;
    for (i = 0; i < sec->stmts_c; i++) {
        try_else(generate_exec_stmt(&sec->exec_stmts[i], out, err, sec, i), CODEGEN_OK, return CODEGEN_ERR);
    }

    
    return CODEGEN_OK;
}

enum codegen_result generate_exec_sections(struct ast_file *file, FILE *out, struct compiler_error *err) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_EXEC_SECTION) {
            try_else(generate_exec_stmts(&file->sections[i].exec_section, out, err), CODEGEN_OK, return CODEGEN_ERR);
        }
    }
    return CODEGEN_OK;
}


enum codegen_result generate_symbol_table(struct hashmap *table, FILE *out) {
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
    err->col = 0;
    err->line = 0;
    strcpy(err->msg, "Unknown error.");
    err->file = file->filename;


    FILE *out = fopen(_out, "wb");
    if (out == NULL) {
        goto _error;
    }


    uint32_t symbol_n = hashmap_get_item_count(&sema->symbol_table);

    try_else(write_int32(out, symbol_n), CODEGEN_OK, goto _error);
    try_else(write_int32(out, sema->exec_len), CODEGEN_OK, goto _error);
    try_else(write_int32(out, sema->data_len), CODEGEN_OK, goto _error);
    


    try_else(generate_symbol_table(&sema->symbol_table, out), CODEGEN_OK, goto _error);

    try_else(generate_exec_sections(file, out, err), CODEGEN_OK, goto _error);

    try_else(generate_data_sections(file, out), CODEGEN_OK, goto _error);


    fclose(out);

    return CODEGEN_OK;

_error:
    fclose(out);
    err->kind = CERROR_CODEGEN;
    
    return CODEGEN_ERR;
}

