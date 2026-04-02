
#include "parser_impl.h"

#include "libs/error_handling.h"
#include "libs/vector/vector.h"




static bool is_matching_kind(struct parser_context *ctx, int offset, enum token_kind kind) {
    if ((ctx->index + offset) >= ctx->n) {
        return false;
    }

    return ctx->in[ctx->index + offset].kind == kind;
}

static bool is_matching_directive(struct parser_context *ctx, int offset, enum directive_token dir) {
    if ((ctx->index + offset) >= ctx->n) {
        return false;
    }
    struct token t = ctx->in[ctx->index + offset];
    return t.kind == TOKEN_DIR && t.dir == dir;
}

static bool is_matching_data_unit(struct parser_context *ctx, int offset, enum data_unit_token data_unit) {
    if ((ctx->index + offset) >= ctx->n) {
        return false;
    }
    struct token t = ctx->in[ctx->index + offset];
    return t.kind == TOKEN_DATA_UNIT && t.data_unit == data_unit;
}

static bool is_matching_punctuation(struct parser_context *ctx, int offset, enum punctuation_token punct) {
    if ((ctx->index + offset) >= ctx->n) {
        return false;
    }
    struct token t = ctx->in[ctx->index + offset];
    return t.kind == TOKEN_PUNCT && t.punct == punct;
}


static void next(struct parser_context *ctx) {
    if ((ctx->index + 1) >= ctx->n) {
        return;
    }

    ctx->index++;
    ctx->col = ctx->in[ctx->index].col;
    
}

enum parser_result copy_terminal(struct parser_context *ctx, struct ast_terminal *term) {
    struct token *t = &ctx->in[ctx->index];

    term->token = t;
    
    return PARSER_OK;
}






static void pop_blank_lines(struct parser_context *ctx) {
    while (is_matching_punctuation(ctx, 0, PUNCT_NEWLINE)) {
        ctx->line++;
        next(ctx);
    }
}





enum parser_result parse_file(struct parser_context *ctx, struct ast_file *file) {

    pop_blank_lines(ctx);
    
    bool _sections = false;


    try_else(parse_sections(ctx, &file->sections, &file->sec_n), PARSER_OK, goto _error);
    _sections = true;
    
    pop_blank_lines(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_EOF)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected EOF");
        goto _error;
    }

    return PARSER_OK;

_error:

    if (_sections) {
        for (uint32_t i = 0; i < file->sec_n; i++) {
            ast_section_deinit(&file->sections[i]);
        }
        free(file->sections);
    }
    

    return PARSER_ERR;
}


static bool follows_section(struct parser_context *ctx) {
    return is_matching_directive(ctx, 0, DIR_DATA)
            || is_matching_directive(ctx, 0, DIR_EXEC);
}

enum parser_result parse_sections(struct parser_context *ctx, struct ast_section **sections, uint32_t *sec_c) {
    
    struct vector sections_v;

    bool _sections = false;
    bool _section = false;

    try_else(vec_init(&sections_v, 10, sizeof(struct ast_section)), VEC_OK, goto _error);
    _sections = true;


    
    struct ast_section section;

    while (follows_section(ctx)) {
        try_else(parse_section(ctx, &section), PARSER_OK, goto _error);
        _section = true;
        try_else(vec_push(&sections_v, &section), VEC_OK, goto _error);
        _section = false;
    }

    *sections = sections_v.ptr;
    *sec_c = sections_v.length;

    return PARSER_OK;

_error:

    if (_sections) {
        vec_deinit(&sections_v, &_ast_section_deinit);
    }

    if (_section) {
        ast_section_deinit(&section);
    }

    return PARSER_ERR;

}

enum parser_result parse_section(struct parser_context *ctx, struct ast_section *section) {
    
    bool _data_section = false;
    bool _exec_section = false;


    if (is_matching_directive(ctx, 0, DIR_DATA)) {
        section->kind = AST_DATA_SECTION;
        try_else(parse_data_section(ctx, &section->data_section), PARSER_OK, goto _error);
        _data_section = true;
    } else if (is_matching_directive(ctx, 0, DIR_EXEC)) {
        section->kind = AST_EXEC_SECTION;
        try_else(parse_exec_section(ctx, &section->exec_section), PARSER_OK, goto _error);
        _exec_section = true;
    } else {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected a section header '.DATA' or '.EXEC'");
        goto _error;
    }

    return PARSER_OK;
    
_error:

    if (_data_section) {
        ast_data_section_deinit(&section->data_section);
    }

    if (_exec_section) {
        ast_exec_section_deinit(&section->exec_section);
    }
    return PARSER_ERR;

}

enum parser_result parse_data_section(struct parser_context *ctx, struct ast_data_section *section) {

    bool _stmts = false;

    try_else(parse_data_dir(ctx), PARSER_OK, goto _error);

    try_else(parse_data_stmts(ctx, &section->data_stmts, &section->stmts_c), PARSER_OK, goto _error);
    _stmts = true;

    return PARSER_OK;

_error:

    if (_stmts) {
        for (uint32_t i = 0; i < section->stmts_c; i++) {
            ast_data_stmt_deinit(&section->data_stmts[i]);
        }
        free(section->data_stmts);
    }

    return PARSER_ERR;
}

enum parser_result parse_data_dir(struct parser_context *ctx) {

    if (!is_matching_directive(ctx, 0, DIR_DATA)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected '.DATA'");
        goto _error;
    }
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_SEMICOLON)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected ':'");
        goto _error;
    }
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_NEWLINE) && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "expected 'end of statement'");
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:

    return PARSER_ERR;
}


bool follows_data_stmt(struct parser_context *ctx) {
    return is_matching_data_unit(ctx, 0, DATA_BYTE)
            || is_matching_data_unit(ctx, 0, DATA_BYTES)
            || is_matching_kind(ctx, 0, TOKEN_IDENT);
}

static bool follows_exec_stmt(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_INSTR)
            //|| is_matching_kind(ctx, 0, TOKEN_MACRO)
            || is_matching_kind(ctx, 0, TOKEN_IDENT)
            || is_matching_directive(ctx, 0, DIR_L)
            || is_matching_directive(ctx, 0, DIR_START);
}

enum parser_result parse_data_stmts(struct parser_context *ctx, struct ast_data_stmt **stmts, uint32_t *stmt_c) {

    bool _stmts = false;
    bool _stmt = false;

    struct vector stmts_v;

    try_else(vec_init(&stmts_v, 5, sizeof(struct ast_data_stmt)), VEC_OK, goto _error);
    _stmts = true;

    struct ast_data_stmt stmt;

    while (follows_data_stmt(ctx)) {
        try_else(parse_data_stmt(ctx, &stmt), PARSER_OK, goto _error);
        _stmt = true;
        try_else(vec_push(&stmts_v, &stmt), VEC_OK, goto _error);
        _stmt = false;
    }
    // Special case error
    if (follows_exec_stmt(ctx)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Executable statement cannot occur in '.DATA' section.");
        goto _error;
    }

    *stmts = stmts_v.ptr;
    *stmt_c = stmts_v.length;

    return PARSER_OK;

_error:
    if (_stmts) {
        vec_deinit(&stmts_v, &_ast_data_stmt_deinit);
    }

    if (_stmt) {
        ast_data_stmt_deinit(&stmt);
    }

    return PARSER_ERR;
}


enum parser_result parse_data_stmt(struct parser_context *ctx, struct ast_data_stmt *stmt) {

    bool _byte_stmt = false;
    bool _bytes_stmt = false;
    //bool _label_stmt = false;

    if (is_matching_data_unit(ctx, 0, DATA_BYTE)) {
        stmt->kind = AST_DATA_STMT_BYTE_STMT;
        try_else(parse_byte_stmt(ctx, &stmt->byte_stmt), PARSER_OK, goto _error);
        _byte_stmt = true;
    } else if (is_matching_data_unit(ctx, 0, DATA_BYTES)) {
        stmt->kind = AST_DATA_STMT_BYTES_STMT;
        try_else(parse_bytes_stmt(ctx, &stmt->bytes_stmt), PARSER_OK, goto _error);
        _bytes_stmt = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        stmt->kind = AST_DATA_STMT_LABEL_STMT;
        try_else(parse_label_stmt(ctx, &stmt->label_stmt), PARSER_OK, goto _error);
        //_label_stmt = true;
    } else {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected statement 'byte', 'bytes', or a label");
        goto _error;
    }

    stmt->line = ctx->line;

    if (!is_matching_punctuation(ctx, 0, PUNCT_NEWLINE) && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "expected 'end of statement'");
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;
    
_error:
    if(_byte_stmt) {
        ast_byte_stmt_deinit(&stmt->byte_stmt);
    }

    if (_bytes_stmt) {
        ast_bytes_stmt_deinit(&stmt->bytes_stmt);
    }

    return PARSER_ERR;
}

static bool follows_initializer(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_NUM)
            || is_matching_kind(ctx, 0, TOKEN_ASCII)
            || is_matching_punctuation(ctx, 0, PUNCT_LBRACE);
}

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct ast_byte_stmt *stmt) {

    bool _init = false;

    if (!is_matching_data_unit(ctx, 0, DATA_BYTE)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected 'byte'");
        goto _error;
    }
    next(ctx);

    if (follows_initializer(ctx)) {
        try_else(parse_initializer(ctx, &stmt->init), PARSER_OK, goto _error);
        _init = true;
        
    } else {
        stmt->init.kind = AST_INIT_NONE;
    }

    

    return PARSER_OK;
_error:

    if (_init) {
        ast_initializer_deinit(&stmt->init);
    }
    return PARSER_ERR;
}

enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct ast_bytes_stmt *stmt) {
    bool _init = false;
    

    if (!is_matching_data_unit(ctx, 0, DATA_BYTES)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected 'bytes'");
        goto _error;
    }
    next(ctx);


    if (!is_matching_punctuation(ctx, 0, PUNCT_LPAR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '('");
        goto _error;
    }
    next(ctx);


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a number");
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->len), PARSER_OK, goto _error);
    next(ctx);
    


    if (!is_matching_punctuation(ctx, 0, PUNCT_RPAR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ')'");
        goto _error;
    }
    next(ctx);


    if (follows_initializer(ctx)) {
        try_else(parse_initializer(ctx, &stmt->init), PARSER_OK, goto _error);
        _init = true;
        
    } else {
        stmt->init.kind = AST_INIT_NONE;
    }

    

    return PARSER_OK;
_error:

    if (_init) {
        ast_initializer_deinit(&stmt->init);
    }

    
    return PARSER_ERR;
}

enum parser_result parse_label_stmt(struct parser_context *ctx, struct ast_label_stmt *stmt) {

    

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->ident), PARSER_OK, goto _error);
    
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_SEMICOLON)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ':'");
        goto _error;
    }
    next(ctx);

   

    return PARSER_OK;
_error:

    
    return PARSER_ERR;
}

enum parser_result parse_initializer(struct parser_context *ctx, struct ast_initializer *init) {

    
    
    bool _byte = false;

    if (is_matching_kind(ctx, 0, TOKEN_NUM)) {
        init->kind = AST_INIT_NUM;
        try_else(copy_terminal(ctx, &init->number), PARSER_OK, goto _error);
        
        next(ctx);
    } else if (is_matching_kind(ctx, 0, TOKEN_ASCII)) {
        init->kind = AST_INIT_ASCII;
        try_else(copy_terminal(ctx, &init->ascii), PARSER_OK, goto _error);
        
        next(ctx);
    } else if (is_matching_punctuation(ctx, 0, PUNCT_LBRACE)) {
        init->kind = AST_INIT_BYTE_INIT;
        try_else(parse_byte_initializer(ctx, &init->byte_init), PARSER_OK, goto _error);
        _byte = true;
        
    } else {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a number, ascii literal or a byte initializer");
        goto _error;
    }

    return PARSER_OK;
_error:

    
    if (_byte) {
        free(init->byte_init.numbers);
    }
    return PARSER_ERR;
}

enum parser_result parse_byte_initializer(struct parser_context *ctx, struct ast_byte_init *byte_init) {

    bool _init = false;

    if (!is_matching_punctuation(ctx, 0, PUNCT_LBRACE)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '{'");
        goto _error;
    }
    next(ctx);

    try_else(parse_numbers(ctx, &byte_init->numbers, &byte_init->num_c), PARSER_OK, goto _error);
    _init = true;
    


    if (!is_matching_punctuation(ctx, 0, PUNCT_RBRACE)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '}'");
        goto _error;
    }
    next(ctx);

    return PARSER_OK;
_error:

    if (_init) {
        free(byte_init->numbers);
    }
    return PARSER_ERR;
}


enum parser_result parse_numbers(struct parser_context *ctx, struct ast_terminal **numbers, uint32_t *byte_c) {

    bool _numbers = false;
    


    struct vector numbers_v;
    try_else(vec_init(&numbers_v,4, sizeof(struct ast_terminal)), VEC_OK, goto _error);
    _numbers = true;

    struct ast_terminal number;

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        *numbers = numbers_v.ptr;
        *byte_c = numbers_v.length;
        return PARSER_OK;
    }

    try_else(copy_terminal(ctx, &number), PARSER_OK, goto _error);
    
    try_else(vec_push(&numbers_v, &number), VEC_OK, goto _error);
    
    next(ctx);

    while (is_matching_punctuation(ctx, 0, PUNCT_COMMA)) {
        next(ctx);

        if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a number after comma");
            goto _error;
        }
        try_else(copy_terminal(ctx, &number), PARSER_OK, goto _error);
        
        try_else(vec_push(&numbers_v, &number), VEC_OK, goto _error);
        
        next(ctx);

    }

    *numbers = numbers_v.ptr;
    *byte_c = numbers_v.length;

    return PARSER_OK;
_error:
    if (_numbers) {
        vec_deinit(&numbers_v, NULL);
    }

    
    return PARSER_ERR;
}

enum parser_result parse_exec_section(struct parser_context *ctx, struct ast_exec_section *section) {
    bool _stmts = false;

    try_else(parse_exec_dir(ctx), PARSER_OK, goto _error);

    try_else(parse_exec_stmts(ctx, &section->exec_stmts, &section->stmts_c), PARSER_OK, goto _error);
    _stmts = true;

    return PARSER_OK;

_error:

    if (_stmts) {
        for (uint32_t i = 0; i < section->stmts_c; i++) {
            ast_exec_stmt_deinit(&section->exec_stmts[i]);
        }
        free(section->exec_stmts);
    }

    return PARSER_ERR;
}

enum parser_result parse_exec_dir(struct parser_context *ctx) {
    if (!is_matching_directive(ctx, 0, DIR_EXEC)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '.EXEC'");
        goto _error;
    }
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_SEMICOLON)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ':'");
        goto _error;
    }
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_NEWLINE) && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "expected 'end of statement'");
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:

    return PARSER_ERR;
}



enum parser_result parse_exec_stmts(struct parser_context *ctx, struct ast_exec_stmt **stmts, uint32_t *stmt_c) {
    bool _stmts = false;
    bool _stmt = false;

    struct vector stmts_v;
    try_else(vec_init(&stmts_v, 5, sizeof(struct ast_exec_stmt)), VEC_OK, goto _error);
    _stmts = true;

    struct ast_exec_stmt stmt;

    while (follows_exec_stmt(ctx)) {
        try_else(parse_exec_stmt(ctx, &stmt), PARSER_OK, goto _error);
        _stmt = true;
        try_else(vec_push(&stmts_v, &stmt), VEC_OK, goto _error);
        _stmt = false;
    }

    if (follows_data_stmt(ctx)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Data statement cannot occur in '.EXEC' section.");
        goto _error;
    }

    *stmts = stmts_v.ptr;
    *stmt_c = stmts_v.length;

    return PARSER_OK;

_error:
    if (_stmts) {
        vec_deinit(&stmts_v, &_ast_exec_stmt_deinit);
    }

    if (_stmt) {
        ast_exec_stmt_deinit(&stmt);
    }

    return PARSER_ERR;
}

enum parser_result parse_exec_stmt(struct parser_context *ctx, struct ast_exec_stmt *stmt) {
    bool _instr_stmt = false;
    //bool _macro_stmt = false;
    

    if (is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        stmt->kind = AST_EXEC_STMT_INSTRUCTION_STMT;
        try_else(parse_instruction_stmt(ctx, &stmt->instruction_stmt), PARSER_OK, goto _error);
        _instr_stmt = true;
    } /*else if (is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        stmt->kind = AST_EXEC_STMT_MACRO_STMT;
        try_else(parse_macro_stmt(ctx, &stmt->macro_stmt), PARSER_OK, goto _error);
        _macro_stmt = true;
    } */
    else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        stmt->kind = AST_EXEC_STMT_LABEL_STMT;
        try_else(parse_label_stmt(ctx, &stmt->label_stmt), PARSER_OK, goto _error);
        
    } else if (is_matching_directive(ctx, 0, DIR_L)) {
        stmt->kind = AST_EXEC_STMT_LOC_LABEL_STMT;
        try_else(parse_loc_label_stmt(ctx, &stmt->loc_label_stmt), PARSER_OK, goto _error);
        
    } else if (is_matching_directive(ctx, 0, DIR_START)) {
        stmt->kind = AST_EXEC_STMT_START_STMT;
        try_else(parse_start_stmt(ctx), PARSER_OK, goto _error);
    } else {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected an instruction, label or a local label statement");
        goto _error;
    }

    stmt->line = ctx->line;

    if (!is_matching_punctuation(ctx, 0, PUNCT_NEWLINE) && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "expected 'end of statement'");
        goto _error;
    }
    pop_blank_lines(ctx);


    return PARSER_OK;
    
_error:
    if(_instr_stmt) {
        ast_instruction_stmt_deinit(&stmt->instruction_stmt);
    }
   

    

    return PARSER_ERR;
}

enum parser_result parse_start_stmt(struct parser_context *ctx) {
    if (!is_matching_directive(ctx, 0, DIR_START)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '.start'");
        goto _error;
    }
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_SEMICOLON)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ':'");
        goto _error;
    }
    next(ctx);

   

    return PARSER_OK;

_error:

    return PARSER_ERR;
}

enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct ast_instruction_stmt *stmt) {

    
    bool _args = false;

    if (!is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected instruction");
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->instr), PARSER_OK, goto _error);
    
    next(ctx);

    if (is_matching_punctuation(ctx, 0, PUNCT_LPAR)) {
        try_else(parse_condition_code(ctx, &stmt->condition_code), PARSER_OK, goto _error);
        
        
    } else {
        stmt->condition_code.token = NULL;
    }

    try_else(parse_args(ctx, &stmt->args, &stmt->args_c), PARSER_OK, goto _error);
    _args = true;

    

    return PARSER_OK;

_error:
    
    
    if (_args) {
        
        free(stmt->args);
    }
    return PARSER_ERR;
}


enum parser_result parse_condition_code(struct parser_context *ctx, struct ast_terminal *cond) {

    

    if (!is_matching_punctuation(ctx, 0, PUNCT_LPAR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '('");
        goto _error;
    }
    next(ctx);


    if (!is_matching_kind(ctx, 0, TOKEN_COND_CODE)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a condition code");
        goto _error;
    }
    try_else(copy_terminal(ctx, cond), PARSER_OK, goto _error);
    next(ctx);
    


    if (!is_matching_punctuation(ctx, 0, PUNCT_RPAR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ')'");
        goto _error;
    }
    next(ctx);


    return PARSER_OK;
_error:

    
    return PARSER_ERR;
}

static bool follows_arg(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_REG) ||
        is_matching_kind(ctx, 0, TOKEN_SYS_REG) ||
        is_matching_kind(ctx, 0, TOKEN_ADDR_REG) ||
        is_matching_kind(ctx, 0, TOKEN_PORT) ||
        is_matching_punctuation(ctx, 0, PUNCT_EQUALS) ||
        is_matching_punctuation(ctx, 0, PUNCT_HASH) ||
        is_matching_directive(ctx, 0, DIR_F) ||
        is_matching_directive(ctx, 0, DIR_B);
}

enum parser_result parse_args(struct parser_context *ctx, struct ast_arg **args, uint32_t *arg_c) {

    bool _args = false;
    

    struct vector args_v;

    try_else(vec_init(&args_v, 4, sizeof(struct ast_arg)), VEC_OK, goto _error);
    _args = true;

    struct ast_arg arg;

    if (!follows_arg(ctx)) {
        *args = args_v.ptr;
        *arg_c = args_v.length;
        return PARSER_OK;
    }

    try_else(parse_arg(ctx, &arg), PARSER_OK, goto _error);
    
    try_else(vec_push(&args_v, &arg), VEC_OK, goto _error);
   
    

    while (is_matching_punctuation(ctx, 0, PUNCT_COMMA)) {
        next(ctx);

        if (!follows_arg(ctx)) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected an argument after a comma");
            goto _error;
        }
        try_else(parse_arg(ctx, &arg), PARSER_OK, goto _error);
        
        try_else(vec_push(&args_v, &arg), VEC_OK, goto _error);
        
       

    }

    *args = args_v.ptr;
    *arg_c = args_v.length;

    return PARSER_OK;
_error:
    if (_args) {
        vec_deinit(&args_v, NULL);
    }

    
    return PARSER_ERR;
}

enum parser_result parse_arg(struct parser_context *ctx, struct ast_arg *arg) {

    


    if (is_matching_punctuation(ctx, 0, PUNCT_EQUALS)) {
        arg->kind = AST_ARG_LABEL;
        try_else(parse_label(ctx, &arg->label), PARSER_OK, goto _error);
        
    } else if (is_matching_punctuation(ctx, 0, PUNCT_HASH)) {
        arg->kind = AST_ARG_IMMEDIATE;
        try_else(parse_immediate(ctx, &arg->immediate), PARSER_OK, goto _error);
        
    } else if (is_matching_directive(ctx, 0, DIR_F) || is_matching_directive(ctx, 0, DIR_B)) {
        arg->kind = AST_ARG_LOC_LABEL;
        try_else(parse_loc_label(ctx, &arg->loc_label), PARSER_OK, goto _error);
        
    } else if (is_matching_kind(ctx, 0, TOKEN_REG)) {
        arg->kind = AST_ARG_REG;
        try_else(copy_terminal(ctx, &arg->reg), PARSER_OK, goto _error);
        next(ctx);
        
    } else if (is_matching_kind(ctx, 0, TOKEN_SYS_REG)) {
       arg->kind = AST_ARG_SYS_REG;
       try_else(copy_terminal(ctx, &arg->sys_reg), PARSER_OK, goto _error);
       next(ctx);
       
    } else if (is_matching_kind(ctx, 0, TOKEN_PORT)) {
       arg->kind = AST_ARG_PORT;
       try_else(copy_terminal(ctx, &arg->port), PARSER_OK, goto _error);
       next(ctx);
       
    } else if (is_matching_kind(ctx, 0, TOKEN_ADDR_REG)) {
       arg->kind = AST_ARG_ADDR_REG;
       try_else(copy_terminal(ctx, &arg->addr_reg), PARSER_OK, goto _error);
       next(ctx);
       
    } else {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected argument.");
        goto _error;
    }

    return PARSER_OK;
_error:
    
    
    return PARSER_ERR;
}

enum parser_result parse_immediate(struct parser_context *ctx, struct ast_terminal *immediate) {

    

    if (!is_matching_punctuation(ctx, 0, PUNCT_HASH)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '#'");
        goto _error;
    }
    next(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a number");
        goto _error;
    }
    try_else(copy_terminal(ctx, immediate), PARSER_OK, goto _error);
    
    next(ctx);


    return PARSER_OK;
_error:
    
    return PARSER_ERR;
}

enum parser_result parse_label(struct parser_context *ctx, struct ast_label *label) {

    
    if (!is_matching_punctuation(ctx, 0, PUNCT_EQUALS)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a '='");
        goto _error;
    }
    next(ctx);
    

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a label afted '='");
        goto _error;
    }
    try_else(copy_terminal(ctx, &label->ident), PARSER_OK, goto _error);
    
    next(ctx);


    return PARSER_OK;
_error:
    
    return PARSER_ERR;
}

enum parser_result parse_loc_label(struct parser_context *ctx, struct ast_loc_label *label) {

    

    

    try_else(parse_direction_dir(ctx, &label->dir), PARSER_OK, goto _error);
    

    if (!is_matching_punctuation(ctx, 0, PUNCT_EQUALS)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a '='");
        goto _error;
    }
    next(ctx);
    

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a label after '='");
        goto _error;
    }
    try_else(copy_terminal(ctx, &label->ident), PARSER_OK, goto _error);
    
    next(ctx);

    return PARSER_OK;
_error:
    
    return PARSER_ERR;
}

enum parser_result parse_direction_dir(struct parser_context *ctx, struct ast_terminal *dir) {

    

    if (!is_matching_directive(ctx, 0, DIR_F) && !is_matching_directive(ctx, 0, DIR_B)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a direction directive ('.f' or '.b')");
        goto _error;
    }
    try_else(copy_terminal(ctx, dir), PARSER_OK, goto _error);
    next(ctx);
    

    return PARSER_OK;
_error:

    
    return PARSER_ERR;
}

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct ast_terminal *dist) {

    

    if (!is_matching_punctuation(ctx, 0, PUNCT_LPAR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '('");
        goto _error;
    }
    next(ctx);


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a number code");
        goto _error;
    }
    try_else(copy_terminal(ctx, dist), PARSER_OK, goto _error);
    
    next(ctx);


    if (!is_matching_punctuation(ctx, 0, PUNCT_RPAR)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ')'");
        goto _error;
    }
    next(ctx);


    return PARSER_OK;
_error:

    
    return PARSER_ERR;
}


enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct ast_loc_label_stmt *stmt) {
    


    if (!is_matching_directive(ctx, 0, DIR_L)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    next(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->ident), PARSER_OK, goto _error);
    
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_SEMICOLON)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected ':'");
        goto _error;
    }
    next(ctx);

    

    return PARSER_OK;
_error:

    
    return PARSER_ERR;
}
