
#include "ast.h"





void _ast_file_deinit(void *node) {
    ast_file_deinit((struct ast_file *)node);
}

void _ast_data_section_deinit(void *node) {
    ast_data_section_deinit((struct ast_data_section *)node);
}

void _ast_exec_section_deinit(void *node) {
    ast_exec_section_deinit((struct ast_exec_section *)node);
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

void _ast_macro_stmt_deinit(void *node) {
    ast_macro_stmt_deinit((struct ast_macro_stmt *)node);
}


void _ast_exec_stmt_deinit(void *node) {
    ast_exec_stmt_deinit((struct ast_exec_stmt *)node);
}





void ast_data_stmt_deinit(struct ast_data_stmt *node) {
    switch (node->kind) {
    case AST_DATA_STMT_BYTE_STMT:
        ast_byte_stmt_deinit(&node->byte_stmt);
    break;

    case AST_DATA_STMT_BYTES_STMT:
        ast_bytes_stmt_deinit(&node->bytes_stmt);
    break;

    case AST_DATA_STMT_LABEL_STMT:
    break;
    }
}


void ast_data_section_deinit(struct ast_data_section *node) {
    for (uint32_t i = 0; i < node->stmts_c; i++) {
        ast_data_stmt_deinit(&node->data_stmts[i]);
    }
    free(node->data_stmts);
}

void ast_exec_stmt_deinit(struct ast_exec_stmt *node) {
    switch (node->kind) {
    case AST_EXEC_STMT_INSTRUCTION_STMT:
        ast_instruction_stmt_deinit(&node->instruction_stmt);
    break;

    case AST_EXEC_STMT_MACRO_STMT:
        ast_macro_stmt_deinit(&node->macro_stmt);
    break;

    case AST_EXEC_STMT_LABEL_STMT:
        
    break;

    case AST_EXEC_STMT_LOC_LABEL_STMT:
       
    break;

    case AST_EXEC_STMT_START_STMT:
    break;
    }
    
}



void ast_exec_section_deinit(struct ast_exec_section *node) {
    for (uint32_t i = 0; i < node->stmts_c; i++) {
        ast_exec_stmt_deinit(&node->exec_stmts[i]);
    }
    free(node->exec_stmts);
}

void ast_section_deinit(struct ast_section *node) {
    if (node->kind == AST_DATA_SECTION) {
        ast_data_section_deinit(&node->data_section);
    } else {
        ast_exec_section_deinit(&node->exec_section);
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
    }
}

void ast_byte_stmt_deinit(struct ast_byte_stmt *node) {
    if (node->is_initialized) {
        ast_initializer_deinit(&node->init);
    }
}

void ast_bytes_stmt_deinit(struct ast_bytes_stmt *node) {
    if (node->is_initialized) {
        ast_initializer_deinit(&node->init);
    }
    
}





void ast_instruction_stmt_deinit(struct ast_instruction_stmt *node) {
    free(node->args);
}

void ast_macro_stmt_deinit(struct ast_macro_stmt *node) {
        
    free(node->args);
}




/*

void null_ast_terminal(struct ast_terminal *node){
    node->lexeme = NULL;
}


void null_ast_file(struct ast_file *node){
    node->sections = null_vector();
}


void null_ast_data_section(struct ast_data_section *node){
    node->data_stmts = null_vector();
}


void null_ast_exec_section(struct ast_exec_section *node){
    node->exec_stmts = null_vector();
}


void null_ast_section(struct ast_section *node){
    null_ast_data_section(&node->data_section);
    null_ast_exec_section(&node->exec_section);
}


void null_ast_byte_initializer(struct ast_byte_initializer *node){
    node->numbers = null_vector();
}


void null_ast_initializer(struct ast_initializer *node){
    null_ast_terminal(&node->ascii);
    null_ast_terminal(&node->number);
    null_ast_byte_initializer(&node->byte_init);
}


void null_ast_byte_stmt(struct ast_byte_stmt *node){
    null_ast_initializer(&node->init);
}


void null_ast_bytes_stmt(struct ast_bytes_stmt *node){
    null_ast_initializer(&node->init);
}


void null_ast_label_stmt(struct ast_label_stmt *node){
    null_ast_terminal(&node->label);
}


void null_ast_data_stmt(struct ast_data_stmt *node){
    byte
}


void null_ast_instruction_stmt(struct ast_instruction_stmt *node){

}


void null_ast_macro_stmt(struct ast_macro_stmt *node){

}


void null_ast_loc_label_stmt(struct ast_loc_label_stmt *node){

}


void null_ast_exec_stmt(struct ast_exec_stmt *node){

}


void null_ast_loc_label(struct ast_loc_label *node){

}


void null_ast_arg(struct ast_arg *node){

}

*/
