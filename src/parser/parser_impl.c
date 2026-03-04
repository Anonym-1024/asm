
#include "parser_impl.h"



static bool is_matching_lexeme(struct parser_context *ctx, int offset, const char *lexeme) {
    if ((ctx->index + offset) >= ctx->n) {
        return false;
    }

    return strcmp(ctx->in[ctx->index + offset].lexeme, lexeme) == 0;
}

static bool is_matching_kind(struct parser_context *ctx, int offset, enum token_kind kind) {
    if ((ctx->index + offset) >= ctx->n) {
        return false;
    }

    return ctx->in[ctx->index + offset].kind == kind;
}


static void next(struct parser_context *ctx) {
    if ((ctx->index + 1) >= ctx->n) {
        return;
    }

    ctx->index++;
    ctx->line = ctx->in[ctx->index].line;
    ctx->col = ctx->in[ctx->index].col;
}

static enum parser_result copy_terminal(struct parser_context *ctx, struct ast_terminal *term) {
    struct token *t = &ctx->in[ctx->index];

    /*term->lexeme = malloc(strlen(t->lexeme) * sizeof(char) + 1);
    if (term->lexeme == NULL) {
        return PARSER_ERR;
    }
    strcpy(term->lexeme, t->lexeme);
    */
    term->line = t->line;
    term->col = t->col;
    term->lexeme = t->lexeme;
    t->lexeme = NULL;
    
    return PARSER_OK;
}


static char *current_lexeme(struct parser_context *ctx) {
    if (strcmp("\n", ctx->in[ctx->index].lexeme) == 0) {
        return "\\n";
    }
    return ctx->in[ctx->index].lexeme;
}




static void pop_blank_lines(struct parser_context *ctx) {
    while (is_matching_lexeme(ctx, 0, "\n")) {
        next(ctx);
    }
}





enum parser_result parse_file(struct parser_context *ctx, struct ast_file *file) {

    pop_blank_lines(ctx);
    
    bool _sections = false;


    try_else(parse_sections(ctx, &file->sections), PARSER_OK, goto _error);
    _sections = true;
    
    pop_blank_lines(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected EOF, found '%.15s' instead: ", current_lexeme(ctx));
        goto _error;
    }

    return PARSER_OK;

_error:

    if (_sections) 
        vec_deinit(&file->sections, &_ast_section_deinit);
    

    return PARSER_ERR;
}


static bool follows_section(struct parser_context *ctx) {
    return is_matching_lexeme(ctx, 0, ".DATA")
            || is_matching_lexeme(ctx, 0, ".EXEC");
}

enum parser_result parse_sections(struct parser_context *ctx, struct vector *sections) {
    
    

    bool _sections = false;
    bool _section = false;

    try_else(vec_init(sections, 10, sizeof(struct ast_section)), VEC_OK, goto _error);
    _sections = true;


    
    struct ast_section section;

    while (follows_section(ctx)) {
        try_else(parse_section(ctx, &section), PARSER_OK, goto _error);
        _section = true;
        try_else(vec_push(sections, &section), VEC_OK, goto _error);
        _section = false;
    }

    

    return PARSER_OK;

_error:

    if (_sections) {
        vec_deinit(sections, &_ast_section_deinit);
    }

    if (_section) {
        ast_section_deinit(&section);
    }

    return PARSER_ERR;

}

enum parser_result parse_section(struct parser_context *ctx, struct ast_section *section) {
    
    bool _data_section = false;
    bool _exec_section = false;


    if (is_matching_lexeme(ctx, 0, ".DATA")) {
        section->kind = AST_DATA_SECTION;
        try_else(parse_data_section(ctx, &section->data_section), PARSER_OK, goto _error);
        _data_section = true;
    } else if (is_matching_lexeme(ctx, 0, ".EXEC")) {
        section->kind = AST_EXEC_SECTION;
        try_else(parse_exec_section(ctx, &section->exec_section), PARSER_OK, goto _error);
        _exec_section = true;
    } else {
        asprintf(&ctx->error_msg, "Expected a section header '.DATA' or '.EXEC', found '%.15s' instead: ", current_lexeme(ctx));
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

    try_else(parse_data_stmts(ctx, &section->data_stmts), PARSER_OK, goto _error);
    _stmts = true;

    return PARSER_OK;

_error:

    if (_stmts) {
        vec_deinit(&section->data_stmts, &_ast_data_stmt_deinit);
    }

    return PARSER_ERR;
}

enum parser_result parse_data_dir(struct parser_context *ctx) {

    if (!is_matching_lexeme(ctx, 0, ".DATA")) {
        asprintf(&ctx->error_msg, "Expected '.DATA', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintf(&ctx->error_msg, "Expected ':', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:

    return PARSER_ERR;
}


bool follows_data_stmt(struct parser_context *ctx) {
    return is_matching_lexeme(ctx, 0, "byte")
            || is_matching_lexeme(ctx, 0, "bytes")
            || is_matching_kind(ctx, 0, TOKEN_IDENT);
}

static bool follows_exec_stmt(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_INSTR)
            || is_matching_kind(ctx, 0, TOKEN_MACRO)
            || is_matching_kind(ctx, 0, TOKEN_IDENT)
            || is_matching_lexeme(ctx, 0, ".l")
            || is_matching_lexeme(ctx, 0, ".start");
}

enum parser_result parse_data_stmts(struct parser_context *ctx, struct vector *stmts) {

    bool _stmts = false;
    bool _stmt = false;


    try_else(vec_init(stmts, 10, sizeof(struct ast_data_stmt)), VEC_OK, goto _error);
    _stmts = true;

    struct ast_data_stmt stmt;

    while (follows_data_stmt(ctx)) {
        try_else(parse_data_stmt(ctx, &stmt), PARSER_OK, goto _error);
        _stmt = true;
        try_else(vec_push(stmts, &stmt), VEC_OK, goto _error);
        _stmt = false;
    }
    // Special case error
    if (follows_exec_stmt(ctx)) {
        asprintf(&ctx->error_msg, "Executable statement cannot occur in '.DATA' section.");
        goto _error;
    }



    return PARSER_OK;

_error:
    if (_stmts) {
        vec_deinit(stmts, &_ast_data_stmt_deinit);
    }

    if (_stmt) {
        ast_data_stmt_deinit(&stmt);
    }

    return PARSER_ERR;
}


enum parser_result parse_data_stmt(struct parser_context *ctx, struct ast_data_stmt *stmt) {

    bool _byte_stmt = false;
    bool _bytes_stmt = false;
    bool _label_stmt = false;

    if (is_matching_lexeme(ctx, 0, "byte")) {
        stmt->kind = AST_DATA_STMT_BYTE_STMT;
        try_else(parse_byte_stmt(ctx, &stmt->byte_stmt), PARSER_OK, goto _error);
        _byte_stmt = true;
    } else if (is_matching_lexeme(ctx, 0, "bytes")) {
        stmt->kind = AST_DATA_STMT_BYTES_STMT;
        try_else(parse_bytes_stmt(ctx, &stmt->bytes_stmt), PARSER_OK, goto _error);
        _bytes_stmt = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        stmt->kind = AST_DATA_STMT_LABEL_STMT;
        try_else(parse_label_stmt(ctx, &stmt->label_stmt), PARSER_OK, goto _error);
        _label_stmt = true;
    } else {
        asprintf(&ctx->error_msg, "Expected statement 'byte', 'bytes', or a label, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }

    return PARSER_OK;
    
_error:
    if(_byte_stmt) {
        ast_byte_stmt_deinit(&stmt->byte_stmt);
    }

    if (_bytes_stmt) {
        ast_bytes_stmt_deinit(&stmt->bytes_stmt);
    }

    if (_label_stmt) {
        ast_label_stmt_deinit(&stmt->label_stmt);
    }

    return PARSER_ERR;
}

static bool follows_initializer(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_NUM)
            || is_matching_kind(ctx, 0, TOKEN_ASCII)
            || is_matching_lexeme(ctx, 0, "{");
}

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct ast_byte_stmt *stmt) {

    bool _init = false;

    if (!is_matching_lexeme(ctx, 0, "byte")) {
        asprintf(&ctx->error_msg, "Expected 'byte', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (follows_initializer(ctx)) {
        try_else(parse_initializer(ctx, &stmt->init), PARSER_OK, goto _error);
        _init = true;
        stmt->is_initialized = true;
    } else {
        stmt->is_initialized = false;
    }

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;
_error:

    if (_init) {
        ast_initializer_deinit(&stmt->init);
    }
    return PARSER_ERR;
}

enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct ast_bytes_stmt *stmt) {
    bool _init = false;
    bool _len = false;

    if (!is_matching_lexeme(ctx, 0, "bytes")) {
        asprintf(&ctx->error_msg, "Expected 'bytes', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    if (!is_matching_lexeme(ctx, 0, "(")) {
        asprintf(&ctx->error_msg, "Expected '(', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        asprintf(&ctx->error_msg, "Expected a number, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->len), PARSER_OK, goto _error);
    next(ctx);
    _len = true;


    if (!is_matching_lexeme(ctx, 0, ")")) {
        asprintf(&ctx->error_msg, "Expected ')', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    if (follows_initializer(ctx)) {
        try_else(parse_initializer(ctx, &stmt->init), PARSER_OK, goto _error);
        _init = true;
        stmt->is_initialized = true;
    } else {
        stmt->is_initialized = false;
    }

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;
_error:

    if (_init) {
        ast_initializer_deinit(&stmt->init);
    }

    if (_len) {
        ast_terminal_deinit(&stmt->len);
    }
    return PARSER_ERR;
}

enum parser_result parse_label_stmt(struct parser_context *ctx, struct ast_label_stmt *stmt) {

    bool _label = false;

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintf(&ctx->error_msg, "Expected label identifier, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->ident), PARSER_OK, goto _error);
    _label = true;
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintf(&ctx->error_msg, "Expected ':', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;
_error:

    if (_label) {
        ast_terminal_deinit(&stmt->ident);
    }
    return PARSER_ERR;
}

enum parser_result parse_initializer(struct parser_context *ctx, struct ast_initializer *init) {

    bool _num = false;
    bool _ascii = false;
    bool _byte = false;

    if (is_matching_kind(ctx, 0, TOKEN_NUM)) {
        init->kind = AST_INIT_NUM;
        try_else(copy_terminal(ctx, &init->number), PARSER_OK, goto _error);
        _num = true;
        next(ctx);
    } else if (is_matching_kind(ctx, 0, TOKEN_ASCII)) {
        init->kind = AST_INIT_ASCII;
        try_else(copy_terminal(ctx, &init->ascii), PARSER_OK, goto _error);
        _ascii = true;
        next(ctx);
    } else if (is_matching_lexeme(ctx, 0, "{")) {
        init->kind = AST_INIT_BYTE_INIT;
        try_else(parse_byte_initializer(ctx, &init->byte_init), PARSER_OK, goto _error);
        _byte = true;
        
    } else {
        asprintf(&ctx->error_msg, "Expected a number, ascii literal or a byte initializer, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }

    return PARSER_OK;
_error:

    if (_num) {
        ast_terminal_deinit(&init->number);
    }

    if (_ascii) {
        ast_terminal_deinit(&init->ascii);
    }

    if (_byte) {
        vec_deinit(&init->byte_init, &_ast_terminal_deinit);
    }
    return PARSER_ERR;
}

enum parser_result parse_byte_initializer(struct parser_context *ctx, struct vector *byte_init) {

    bool _init = false;

    if (!is_matching_lexeme(ctx, 0, "{")) {
        asprintf(&ctx->error_msg, "Expected '{', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    try_else(parse_numbers(ctx, byte_init), PARSER_OK, goto _error);
    _init = true;
    


    if (!is_matching_lexeme(ctx, 0, "}")) {
        asprintf(&ctx->error_msg, "Expected '}', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    return PARSER_OK;
_error:

    if (_init) {
        vec_deinit(byte_init, &_ast_terminal_deinit);
    }
    return PARSER_ERR;
}


enum parser_result parse_numbers(struct parser_context *ctx, struct vector *numbers) {

    bool _numbers = false;
    bool _number = false;

    try_else(vec_init(numbers,4, sizeof(struct ast_terminal)), VEC_OK, goto _error);
    _numbers = true;

    struct ast_terminal number;

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        return PARSER_OK;
    }

    try_else(copy_terminal(ctx, &number), PARSER_OK, goto _error);
    _number = true;
    try_else(vec_push(numbers, &number), VEC_OK, goto _error);
    _number = false;
    next(ctx);

    while (is_matching_lexeme(ctx, 0, ",")) {
        next(ctx);

        if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
            asprintf(&ctx->error_msg, "Expected a number after comma, found '%.15s' instead.", current_lexeme(ctx));
            goto _error;
        }
        try_else(copy_terminal(ctx, &number), PARSER_OK, goto _error);
        _number = true;
        try_else(vec_push(numbers, &number), VEC_OK, goto _error);
        _number = false;
        next(ctx);

    }

    return PARSER_OK;
_error:
    if (_numbers) {
        vec_deinit(numbers, &_ast_terminal_deinit);
    }

    if (_number) {
        ast_terminal_deinit(&number);
    }
    return PARSER_ERR;
}

enum parser_result parse_exec_section(struct parser_context *ctx, struct ast_exec_section *section) {
    bool _stmts = false;

    try_else(parse_exec_dir(ctx), PARSER_OK, goto _error);

    try_else(parse_exec_stmts(ctx, &section->exec_stmts), PARSER_OK, goto _error);
    _stmts = true;

    return PARSER_OK;

_error:

    if (_stmts) {
        vec_deinit(&section->exec_stmts, &_ast_exec_stmt_deinit);
    }

    return PARSER_ERR;
}

enum parser_result parse_exec_dir(struct parser_context *ctx) {
    if (!is_matching_lexeme(ctx, 0, ".EXEC")) {
        asprintf(&ctx->error_msg, "Expected '.EXEC', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintf(&ctx->error_msg, "Expected ':', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:

    return PARSER_ERR;
}



enum parser_result parse_exec_stmts(struct parser_context *ctx, struct vector *stmts) {
    bool _stmts = false;
    bool _stmt = false;


    try_else(vec_init(stmts, 10, sizeof(struct ast_exec_stmt)), VEC_OK, goto _error);
    _stmts = true;

    struct ast_exec_stmt stmt;

    while (follows_exec_stmt(ctx)) {
        try_else(parse_exec_stmt(ctx, &stmt), PARSER_OK, goto _error);
        _stmt = true;
        try_else(vec_push(stmts, &stmt), VEC_OK, goto _error);
        _stmt = false;
    }

    if (follows_data_stmt(ctx)) {
        asprintf(&ctx->error_msg, "Data statement cannot occur in '.EXEC' section.");
        goto _error;
    }


    return PARSER_OK;

_error:
    if (_stmts) {
        vec_deinit(stmts, &_ast_exec_stmt_deinit);
    }

    if (_stmt) {
        ast_exec_stmt_deinit(&stmt);
    }

    return PARSER_ERR;
}

enum parser_result parse_exec_stmt(struct parser_context *ctx, struct ast_exec_stmt *stmt) {
     bool _instr_stmt = false;
    bool _macro_stmt = false;
    bool _label_stmt = false;
    bool _loc_label_stmt = false;

    if (is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        stmt->kind = AST_EXEC_STMT_INSTRUCTION_STMT;
        try_else(parse_instruction_stmt(ctx, &stmt->instruction_stmt), PARSER_OK, goto _error);
        _instr_stmt = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        stmt->kind = AST_EXEC_STMT_MACRO_STMT;
        try_else(parse_macro_stmt(ctx, &stmt->macro_stmt), PARSER_OK, goto _error);
        _macro_stmt = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        stmt->kind = AST_EXEC_STMT_LABEL_STMT;
        try_else(parse_label_stmt(ctx, &stmt->label_stmt), PARSER_OK, goto _error);
        _label_stmt = true;
    } else if (is_matching_lexeme(ctx, 0, ".l")) {
        stmt->kind = AST_EXEC_STMT_LOC_LABEL_STMT;
        try_else(parse_loc_label_stmt(ctx, &stmt->loc_label_stmt), PARSER_OK, goto _error);
        _loc_label_stmt = true;
    } else if (is_matching_lexeme(ctx, 0, ".start")) {
        stmt->kind = AST_EXEC_STMT_START_STMT;
        try_else(parse_start_stmt(ctx), PARSER_OK, goto _error);
    } else {
        asprintf(&ctx->error_msg, "Expected an instruction, macro, label or a local label statement, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }


    


    return PARSER_OK;
    
_error:
    if(_instr_stmt) {
        ast_instruction_stmt_deinit(&stmt->instruction_stmt);
    }

    if (_macro_stmt) {
        ast_macro_stmt_deinit(&stmt->macro_stmt);
    }

    if (_label_stmt) {
        ast_label_stmt_deinit(&stmt->label_stmt);
    }

    if (_loc_label_stmt) {
        ast_loc_label_stmt_deinit(&stmt->loc_label_stmt);
    }

    return PARSER_ERR;
}

enum parser_result parse_start_stmt(struct parser_context *ctx) {
    if (!is_matching_lexeme(ctx, 0, ".start")) {
        asprintf(&ctx->error_msg, "Expected '.start', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintf(&ctx->error_msg, "Expected ':', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:

    return PARSER_ERR;
}

enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct ast_instruction_stmt *stmt) {

    bool _instr = false;
    bool _cond = false;
    bool _args = false;

    if (!is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        asprintf(&ctx->error_msg, "Expected instruction, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->instr), PARSER_OK, goto _error);
    _instr = true;
    next(ctx);

    if (is_matching_lexeme(ctx, 0, "(")) {
        try_else(parse_condition_code(ctx, &stmt->condition_code), PARSER_OK, goto _error);
        _cond = true;
        stmt->is_conditional = true;
    } else {
        stmt->is_conditional = false;
    }

    try_else(parse_args(ctx, &stmt->args), PARSER_OK, goto _error);
    _args = true;

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:
    if (_instr) {
        ast_terminal_deinit(&stmt->instr);
    }

    if (_cond) {
        ast_terminal_deinit(&stmt->condition_code);
    }
    if (_args) {
        vec_deinit(&stmt->args, &_ast_arg_deinit);
    }
    return PARSER_ERR;
}


enum parser_result parse_condition_code(struct parser_context *ctx, struct ast_terminal *cond) {

    bool _cond = false;

    if (!is_matching_lexeme(ctx, 0, "(")) {
        asprintf(&ctx->error_msg, "Expected '(', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    if (!is_matching_kind(ctx, 0, TOKEN_COND_CODE)) {
        asprintf(&ctx->error_msg, "Expected a condition code, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, cond), PARSER_OK, goto _error);
    next(ctx);
    _cond = true;


    if (!is_matching_lexeme(ctx, 0, ")")) {
        asprintf(&ctx->error_msg, "Expected ')', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    return PARSER_OK;
_error:

    if (_cond) {
        ast_terminal_deinit(cond);
    }
    return PARSER_ERR;
}

static bool follows_arg(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_REG) ||
        is_matching_kind(ctx, 0, TOKEN_SYS_REG) ||
        is_matching_kind(ctx, 0, TOKEN_ADDR_REG) ||
        is_matching_kind(ctx, 0, TOKEN_PORT) ||
        is_matching_kind(ctx, 0, TOKEN_IDENT) ||
        is_matching_lexeme(ctx, 0, "#") ||
        is_matching_lexeme(ctx, 0, ".f") ||
        is_matching_lexeme(ctx, 0, ".b");
}

enum parser_result parse_args(struct parser_context *ctx, struct vector *args) {

    bool _args = false;
    bool _arg = false;

    try_else(vec_init(args, 3, sizeof(struct ast_arg)), VEC_OK, goto _error);
    _args = true;

    struct ast_arg arg;

    if (!follows_arg(ctx)) {
        return PARSER_OK;
    }

    try_else(parse_arg(ctx, &arg), PARSER_OK, goto _error);
    _arg = true;
    try_else(vec_push(args, &arg), VEC_OK, goto _error);
    _arg = false;
    

    while (is_matching_lexeme(ctx, 0, ",")) {
        next(ctx);

        if (!follows_arg(ctx)) {
            asprintf(&ctx->error_msg, "Expected an argument after a comma, found '%.15s' instead.", current_lexeme(ctx));
            goto _error;
        }
        try_else(parse_arg(ctx, &arg), PARSER_OK, goto _error);
        _arg = true;
        try_else(vec_push(args, &arg), VEC_OK, goto _error);
        _arg = false;
       

    }

    return PARSER_OK;
_error:
    if (_args) {
        vec_deinit(args, &_ast_arg_deinit);
    }

    if (_arg) {
        ast_arg_deinit(&arg);
    }
    return PARSER_ERR;
}

enum parser_result parse_arg(struct parser_context *ctx, struct ast_arg *arg) {

    bool _label = false;
    bool _immediate = false;
    bool _loc_label = false;
    bool _reg = false;
    bool _sys_reg = false;
    bool _port = false;
    bool _addr_reg = false;


    if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        arg->kind = AST_ARG_LABEL;
        try_else(parse_label(ctx, &arg->label), PARSER_OK, goto _error);
        _label = true;
    } else if (is_matching_lexeme(ctx, 0, "#")) {
        arg->kind = AST_ARG_IMMEDIATE;
        try_else(parse_immediate(ctx, &arg->immediate), PARSER_OK, goto _error);
        _immediate = true;
    } else if (is_matching_lexeme(ctx, 0, ".f") || is_matching_lexeme(ctx, 0, ".b")) {
        arg->kind = AST_ARG_LOC_LABEL;
        try_else(parse_loc_label(ctx, &arg->loc_label), PARSER_OK, goto _error);
        _loc_label = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_REG)) {
        arg->kind = AST_ARG_REG;
        try_else(copy_terminal(ctx, &arg->reg), PARSER_OK, goto _error);
        next(ctx);
        _reg = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_SYS_REG)) {
       arg->kind = AST_ARG_SYS_RES;
       try_else(copy_terminal(ctx, &arg->sys_reg), PARSER_OK, goto _error);
       next(ctx);
       _sys_reg = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_PORT)) {
       arg->kind = AST_ARG_PORT;
       try_else(copy_terminal(ctx, &arg->port), PARSER_OK, goto _error);
       next(ctx);
       _port = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_ADDR_REG)) {
       arg->kind = AST_ARG_ADDR_REG;
       try_else(copy_terminal(ctx, &arg->addr_reg), PARSER_OK, goto _error);
       next(ctx);
       _addr_reg = true;
    } else {
        asprintf(&ctx->error_msg, "Expected argument, found '%s' instead.", current_lexeme(ctx));
        goto _error;
    }

    return PARSER_OK;
_error:
    if (_addr_reg) {
        ast_terminal_deinit(&arg->addr_reg);
    }
    if (_immediate) {
        ast_terminal_deinit(&arg->immediate);
    }
    if (_label) {
        ast_terminal_deinit(&arg->label);
    }
    if (_loc_label) {
        ast_loc_label_deinit(&arg->loc_label);
    }
    if (_port) {    
        ast_terminal_deinit(&arg->port);
    }
    if (_reg) {
        ast_terminal_deinit(&arg->reg);
    }
    if (_sys_reg) {
        ast_terminal_deinit(&arg->sys_reg);
    }
    
    return PARSER_ERR;
}

enum parser_result parse_immediate(struct parser_context *ctx, struct ast_terminal *immediate) {

    bool _num = false;

    if (!is_matching_lexeme(ctx, 0, "#")) {
        asprintf(&ctx->error_msg, "Expected '#', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        asprintf(&ctx->error_msg, "Expected a number, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, immediate), PARSER_OK, goto _error);
    _num = true;
    next(ctx);


    return PARSER_OK;
_error:
    if (_num) {
        ast_terminal_deinit(immediate);
    }
    return PARSER_ERR;
}

enum parser_result parse_label(struct parser_context *ctx, struct ast_terminal *label) {

    bool _ident = false;


    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintf(&ctx->error_msg, "Expected a label, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, label), PARSER_OK, goto _error);
    _ident = true;
    next(ctx);


    return PARSER_OK;
_error:
    if (_ident) {
        ast_terminal_deinit(label);
    }
    return PARSER_ERR;
}

enum parser_result parse_loc_label(struct parser_context *ctx, struct ast_loc_label *label) {

    bool _dir = false;
    bool _dist = false;
    bool _ident = false;

    

    try_else(parse_direction_dir(ctx, &label->dir), PARSER_OK, goto _error);
    _dir = true;

    if (is_matching_lexeme(ctx, 0, "(")) {
        try_else(parse_loc_label_dist(ctx, &label->dist), PARSER_OK, goto _error);
        _dist = true;
        label->has_dist = true;
    } else {
        label->has_dist = false;
    }
    

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintf(&ctx->error_msg, "Expected a label, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, &label->ident), PARSER_OK, goto _error);
    _ident = true;
    next(ctx);

    return PARSER_OK;
_error:
    if (_dir) {
        ast_terminal_deinit(&label->dir);
    }

    if (_dist) {
        ast_terminal_deinit(&label->dist);
    }

    if (_ident) {
        ast_terminal_deinit(&label->ident);
    }
    return PARSER_ERR;
}

enum parser_result parse_direction_dir(struct parser_context *ctx, struct ast_terminal *dir) {

    bool _dir = false;

    if (!is_matching_lexeme(ctx, 0, ".f") && !is_matching_lexeme(ctx, 0, ".b")) {
        asprintf(&ctx->error_msg, "Expected a direction directive ('.f' or '.b'), found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, dir), PARSER_OK, goto _error);
    next(ctx);
    _dir = true;

    return PARSER_OK;
_error:

    if (_dir) {
        ast_terminal_deinit(dir);
    }
    return PARSER_ERR;
}

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct ast_terminal *dist) {

    bool _dist = false;

    if (!is_matching_lexeme(ctx, 0, "(")) {
        asprintf(&ctx->error_msg, "Expected '(', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        asprintf(&ctx->error_msg, "Expected a number code, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, dist), PARSER_OK, goto _error);
    _dist = true;
    next(ctx);


    if (!is_matching_lexeme(ctx, 0, ")")) {
        asprintf(&ctx->error_msg, "Expected ')', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);


    return PARSER_OK;
_error:

    if (_dist) {
        ast_terminal_deinit(dist);
    }
    return PARSER_ERR;
}

enum parser_result parse_macro_stmt(struct parser_context *ctx, struct ast_macro_stmt *stmt) {
     bool _macro = false;
    bool _cond = false;
    bool _args = false;

    if (!is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        asprintf(&ctx->error_msg, "Expected instruction, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->instr), PARSER_OK, goto _error);
    _macro = true;
    next(ctx);

    if (is_matching_lexeme(ctx, 0, "(")) {
        try_else(parse_condition_code(ctx, &stmt->condition_code), PARSER_OK, goto _error);
        _cond = true;
        stmt->is_conditional = true;
    } else {
        stmt->is_conditional = false;
    }

    try_else(parse_args(ctx, &stmt->args), PARSER_OK, goto _error);
    _args = true;

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;

_error:
    if (_macro) {
        ast_terminal_deinit(&stmt->instr);
    }

    if (_cond) {
        ast_terminal_deinit(&stmt->condition_code);
    }
    if (_args) {
        vec_deinit(&stmt->args, &_ast_arg_deinit);
    }
    return PARSER_ERR;
}

enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct ast_loc_label_stmt *stmt) {
    bool _label = false;


    if (!is_matching_lexeme(ctx, 0, ".l")) {
        asprintf(&ctx->error_msg, "Expected label identifier, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintf(&ctx->error_msg, "Expected label identifier, found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    try_else(copy_terminal(ctx, &stmt->ident), PARSER_OK, goto _error);
    _label = true;
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintf(&ctx->error_msg, "Expected ':', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    next(ctx);

    if (!is_matching_lexeme(ctx, 0, "\n") && !is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintf(&ctx->error_msg, "Expected '\\n', found '%.15s' instead.", current_lexeme(ctx));
        goto _error;
    }
    pop_blank_lines(ctx);

    return PARSER_OK;
_error:

    if (_label) {
        ast_terminal_deinit(&stmt->ident);
    }
    return PARSER_ERR;
}
