
#include "ast.h"





void _ast_file_deinit(void *node) {
    ast_file_deinit((struct ast_file *)node);
}

void _ast_data_section_deinit(void *node) {
    ast_data_section_deinit((struct ast_data_section *)node);
}

void _ast_code_section_deinit(void *node) {
    ast_code_section_deinit((struct ast_code_section *)node);
}

void _ast_section_deinit(void *node) {
    ast_section_deinit((struct ast_section *)node);
}

void _ast_initializer_deinit(void *node) {
    ast_initializer_deinit((struct ast_initializer *)node);
}

void _ast_byte_stmt_deinit(void *node) {
    ast_byte_stmt_deinit((struct ast_byte_stmt *)node);
}

void _ast_bytes_stmt_deinit(void *node) {
    ast_bytes_stmt_deinit((struct ast_bytes_stmt *)node);

}


void _ast_data_stmt_deinit(void *node) {
    ast_data_stmt_deinit((struct ast_data_stmt *)node);
}

void _ast_instruction_stmt_deinit(void *node) {
    ast_instruction_stmt_deinit((struct ast_instruction_stmt *)node);
}



void _ast_code_stmt_deinit(void *node) {
    ast_code_stmt_deinit((struct ast_code_stmt *)node);
}

void _ast_head_section_deinit(void *node) {
    ast_head_section_deinit((struct ast_head_section*)node);
}



void ast_data_stmt_deinit(struct ast_data_stmt *node) {
    switch (node->kind) {
    case AST_DATA_STMT_BYTE:
        ast_byte_stmt_deinit(&node->byte_stmt);
    break;

    case AST_DATA_STMT_BYTES:
        ast_bytes_stmt_deinit(&node->bytes_stmt);
    break;

    case AST_DATA_STMT_LABEL:
    break;
    }
}


void ast_data_section_deinit(struct ast_data_section *node) {
    for (uint32_t i = 0; i < node->stmts_c; i++) {
        ast_data_stmt_deinit(&node->data_stmts[i]);
    }
    free(node->data_stmts);
}

void ast_code_stmt_deinit(struct ast_code_stmt *node) {
    switch (node->kind) {
    case AST_CODE_STMT_INSTRUCTION:
        ast_instruction_stmt_deinit(&node->instruction_stmt);
    break;


    case AST_CODE_STMT_LABEL:
        
    break;

    case AST_CODE_STMT_LOC_LABEL:
       
    break;

    case AST_CODE_STMT_START:
    break;
    }
    
}



void ast_code_section_deinit(struct ast_code_section *node) {
    for (uint32_t i = 0; i < node->stmts_c; i++) {
        ast_code_stmt_deinit(&node->code_stmts[i]);
    }
    free(node->code_stmts);
}

void ast_section_deinit(struct ast_section *node) {
    if (node->kind == AST_DATA_SECTION) {
        ast_data_section_deinit(&node->data_section);
    } else {
        ast_code_section_deinit(&node->code_section);
    }
}



void ast_file_deinit(struct ast_file *node) {
    for (uint32_t i = 0; i < node->sec_n; i++) {
        ast_section_deinit(&node->sections[i]);
    }
    free(node->sections);
}





void ast_initializer_deinit(struct ast_initializer *node) {
    switch (node->kind) {
    case AST_INIT_ASCII:
        
    break;

    case AST_INIT_NUM:
       
    break;

    case AST_INIT_BYTE_INIT:
        free(node->byte_init.numbers);
    break;
    case AST_INIT_NONE:
    break;
    }
}

void ast_byte_stmt_deinit(struct ast_byte_stmt *node) {
    if (node->init.kind != AST_INIT_NONE) {
        ast_initializer_deinit(&node->init);
    }
}

void ast_bytes_stmt_deinit(struct ast_bytes_stmt *node) {
    if (node->init.kind != AST_INIT_NONE) {
        ast_initializer_deinit(&node->init);
    }
    
}





void ast_instruction_stmt_deinit(struct ast_instruction_stmt *node) {
    free(node->args);
}




void ast_head_section_deinit(struct ast_head_section *node) {
    free(node->stmts);
}