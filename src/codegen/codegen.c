 
#include "codegen.h"




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

static void add_imm(uint8_t *byte2, uint8_t *byte3, uint16_t imm) {
    *byte3 |= imm;
    *byte2 |= (imm >> 8);
}

static void generate_instruction(struct ast_instruction_stmt *instr, FILE *out) {
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
                add_imm(&byte2, &byte3, arg->label.offset);
                break;
            case AST_ARG_LOC_LABEL:
                byte1 |= 0x10;
                add_imm(&byte2, &byte3, arg->loc_label.offset);
                break;
            case AST_ARG_IMMEDIATE:
                byte1 |= 0x10;
                add_imm(&byte2, &byte3, arg->immediate.token->number);
                break;
        }
    }

    fwrite(&byte0, sizeof(uint8_t), 1, out);
    fwrite(&byte1, sizeof(uint8_t), 1, out);
    fwrite(&byte2, sizeof(uint8_t), 1, out);
    fwrite(&byte3, sizeof(uint8_t), 1, out);

}

static void generate_start_instruction(uint16_t start, FILE *out) {
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;
    uint8_t byte2 = 0;
    uint8_t byte3 = 0;

    uint32_t opcode = INSTR_BR << 1;


    byte0 |= (opcode & (0xF0)) >> 4;
    byte1 |= (opcode & (0x0F)) << 4;

    
    add_imm(&byte2, &byte3, start);
    
    

    fwrite(&byte0, sizeof(uint8_t), 1, out);
    fwrite(&byte1, sizeof(uint8_t), 1, out);
    fwrite(&byte2, sizeof(uint8_t), 1, out);
    fwrite(&byte3, sizeof(uint8_t), 1, out);

}

static void generate_exec_section(struct ast_exec_section *sec, FILE *out) {
    for (uint32_t i = 0; i < sec->stmts_c; i++) {
        if (sec->exec_stmts[i].kind == AST_EXEC_STMT_INSTRUCTION_STMT) {
            generate_instruction(&sec->exec_stmts[i].instruction_stmt, out);
        }
    }
}

static void generate_exec_sections(struct ast_file *file, FILE *out) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_EXEC_SECTION) {
            generate_exec_section(&file->sections[i].exec_section, out);
        }
    }
}


static void generate_byte_stmt(struct ast_byte_stmt *byte, FILE *out) {
    uint8_t value = 0;
    if (byte->init.kind == AST_INIT_NUM) {
        value = byte->init.number.token->number;
    }
    fwrite(&value, sizeof(uint8_t), 1, out);
}


static void generate_bytes_stmt(struct ast_bytes_stmt *bytes, FILE *out) {
    
    if (bytes->init.kind == AST_INIT_NONE) {
        for(uint32_t i = 0; i < (uint32_t)bytes->len.token->number; i++) {
            uint8_t zero = 0;
            fwrite(&zero, sizeof(uint8_t), 1, out);
        }
    } else if (bytes->init.kind == AST_INIT_NUM) {
        for(uint32_t i = 0; i < (uint32_t)bytes->len.token->number; i++) {
            uint8_t value = 0;
            if (i<sizeof(uint32_t)) {
                value = (bytes->init.number.token->number >> (8 * i)) & 0xFF;
            }
            fwrite(&value, sizeof(uint8_t), 1, out);
        }
    } else if (bytes->init.kind == AST_INIT_BYTE_INIT) {
        for (uint32_t i = 0; i < (uint32_t)bytes->len.token->number; i++) {
            uint8_t value = 0;
            if (i < bytes->init.byte_init.num_c) {
                value = bytes->init.byte_init.numbers[i].token->number;
            }
            fwrite(&value, sizeof(uint8_t), 1, out);
        }
    } else if (bytes->init.kind == AST_INIT_ASCII) {
        size_t ascii_len = strlen(bytes->init.ascii.token->lexeme);
        for (uint32_t i = 0; i < (uint32_t)bytes->len.token->number; i++) {
            uint8_t value = 0;
            if (i < ascii_len) {
                value = bytes->init.ascii.token->lexeme[i];
            }
            fwrite(&value, sizeof(uint8_t), 1, out);
        }
    }
    
}

static void generate_data_section(struct ast_data_section *sec, FILE *out) {
    for (uint32_t i = 0; i < sec->stmts_c; i++) {
        switch (sec->data_stmts[i].kind) {
        case AST_DATA_STMT_BYTE_STMT:
            generate_byte_stmt(&sec->data_stmts[i].byte_stmt, out);
        break;

        case AST_DATA_STMT_BYTES_STMT:
            generate_bytes_stmt(&sec->data_stmts[i].bytes_stmt, out);
        break;

        default:
        break;
        }
    }
}

static void generate_data_sections(struct ast_file *file, FILE *out) {
    for (uint32_t i = 0; i < file->sec_n; i++) {
        if (file->sections[i].kind == AST_DATA_SECTION) {
            generate_data_section(&file->sections[i].data_section, out);
        }
    }
}






void generate_binary(struct ast_file *file, uint16_t start, FILE *out) {

    
    
    generate_start_instruction(start,  out);

    generate_exec_sections(file, out);
    generate_data_sections(file, out);
}
