
#include "ast.h"



void _ast_terminal_deinit(void *node) {
    ast_terminal_deinit((struct ast_terminal *)node);
}

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

void _ast_label_stmt_deinit(void *node) {
    ast_label_stmt_deinit((struct ast_label_stmt *)node);
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

void _ast_loc_label_stmt_deinit(void *node) {
    ast_loc_label_stmt_deinit((struct ast_loc_label_stmt *)node);
}

void _ast_exec_stmt_deinit(void *node) {
    ast_exec_stmt_deinit((struct ast_exec_stmt *)node);
}

void _ast_loc_label_deinit(void *node) {
    ast_loc_label_deinit((struct ast_loc_label *)node);
}

void _ast_arg_deinit(void *node) {
    ast_arg_deinit((struct ast_arg *)node);
}


void ast_terminal_deinit(struct ast_terminal *node) {
    free(node->lexeme);
    node->lexeme = NULL;
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
        ast_label_stmt_deinit(&node->label_stmt);
    break;
    }
}


void ast_data_section_deinit(struct ast_data_section *node) {
    vec_deinit(&node->data_stmts, &_ast_data_stmt_deinit);
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
        ast_label_stmt_deinit(&node->label_stmt);
    break;

    case AST_EXEC_STMT_LOC_LABEL_STMT:
        ast_loc_label_stmt_deinit(&node->loc_label_stmt);
    break;

    case AST_EXEC_STMT_START_STMT:
    break;
    }
}



void ast_exec_section_deinit(struct ast_exec_section *node) {
    vec_deinit(&node->exec_stmts, &_ast_exec_stmt_deinit);
}

void ast_section_deinit(struct ast_section *node) {
    if (node->kind == AST_DATA_SECTION) {
        ast_data_section_deinit(&node->data_section);
    } else {
        ast_exec_section_deinit(&node->exec_section);
    }
}



void ast_file_deinit(struct ast_file *node) {
    vec_deinit(&node->sections, &_ast_section_deinit);
}




void numbers_deinit(void *p) {
    free(p);
}

void ast_initializer_deinit(struct ast_initializer *node) {
    switch (node->kind) {
    case AST_INIT_ASCII:
        ast_terminal_deinit(&node->ascii);
    break;

    case AST_INIT_NUM:
        ast_terminal_deinit(&node->number);
    break;

    case AST_INIT_BYTE_INIT:
        vec_deinit(&node->byte_init, &_ast_terminal_deinit);
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
    ast_terminal_deinit(&node->len);
}

void ast_label_stmt_deinit(struct ast_label_stmt *node) {
    ast_terminal_deinit(&node->ident);
}




void ast_instruction_stmt_deinit(struct ast_instruction_stmt *node) {
    ast_terminal_deinit(&node->instr);
    if (node->is_conditional) {
        ast_terminal_deinit(&node->condition_code);
    }

    vec_deinit(&node->args, &_ast_arg_deinit);
}

void ast_macro_stmt_deinit(struct ast_macro_stmt *node) {
    ast_terminal_deinit(&node->instr);
    if (node->is_conditional) {
        ast_terminal_deinit(&node->condition_code);
    }

    vec_deinit(&node->args, &_ast_arg_deinit);
}

void ast_loc_label_stmt_deinit(struct ast_loc_label_stmt *node) {
    ast_terminal_deinit(&node->ident);
}

void ast_loc_label_deinit(struct ast_loc_label *node) {
    ast_terminal_deinit(&node->ident);
    ast_terminal_deinit(&node->dir);
    if (node->has_dist) {
        ast_terminal_deinit(&node->dist);
    }
}

void ast_arg_deinit(struct ast_arg *node) {
    switch (node->kind) {
        case AST_ARG_REG:
            ast_terminal_deinit(&node->reg);
        break;

        case AST_ARG_SYS_RES:
            ast_terminal_deinit(&node->sys_reg);
        break;

        case AST_ARG_ADDR_REG:
            ast_terminal_deinit(&node->addr_reg);
        break;

        case AST_ARG_PORT:
            ast_terminal_deinit(&node->port);
        break;

        case AST_ARG_IMMEDIATE:
            ast_terminal_deinit(&node->immediate);
        break;

        case AST_ARG_LABEL:
            ast_terminal_deinit(&node->label);
        break;

        case AST_ARG_LOC_LABEL:
            ast_loc_label_deinit(&node->loc_label);
        break;

    }
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
