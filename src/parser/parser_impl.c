
#include "parser_impl.h"

#include "error/compiler_error.h"
#include "libs/error_handling.h"
#include "libs/vector/vector.h"
#include "shared/ast.h"
#include "shared/token.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>




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


static void copy_terminal(struct parser_context *ctx, struct ast_terminal *term) {
    struct token *t = &ctx->in[ctx->index];

    term->token = t;
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
            || is_matching_directive(ctx, 0, DIR_CODE)
            || is_matching_directive(ctx, 0, DIR_HEAD);
}

enum parser_result parse_sections(struct parser_context *ctx, struct ast_section **sections, uint32_t *sec_n) {

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
    *sec_n = sections_v.length;

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
    bool _code_section = false;
    bool _head_section = false;


    if (is_matching_directive(ctx, 0, DIR_DATA)) {
        section->kind = AST_DATA_SECTION;
        try_else(parse_data_section(ctx, &section->data_section), PARSER_OK, goto _error);
        _data_section = true;
    } else if (is_matching_directive(ctx, 0, DIR_CODE)) {
        section->kind = AST_CODE_SECTION;
        try_else(parse_code_section(ctx, &section->code_section), PARSER_OK, goto _error);
        _code_section = true;
    } else if (is_matching_directive(ctx, 0, DIR_HEAD)) {
        section->kind = AST_HEAD_SECTION;
        try_else(parse_head_section(ctx, &section->head_section), PARSER_OK, goto _error);
        _head_section = true;
    } else {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected a section header '.DATA' or '.CODE'");
        goto _error;
    }

    return PARSER_OK;

_error:

    if (_data_section) {
        ast_data_section_deinit(&section->data_section);
    }

    if (_code_section) {
        ast_code_section_deinit(&section->code_section);
    }

    if (_head_section) {
        ast_head_section_deinit(&section->head_section);
    }

    return PARSER_ERR;

}

enum parser_result parse_data_section(struct parser_context *ctx, struct ast_data_section *section) {

    bool _stmts = false;

    try_else(parse_data_dir(ctx), PARSER_OK, goto _error);

    try_else(parse_data_stmts(ctx, &section->data_stmts, &section->stmts_n), PARSER_OK, goto _error);
    _stmts = true;

    return PARSER_OK;

_error:

    if (_stmts) {
        for (uint32_t i = 0; i < section->stmts_n; i++) {
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

static bool follows_code_stmt(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_INSTR)
            //|| is_matching_kind(ctx, 0, TOKEN_MACRO)
            || is_matching_kind(ctx, 0, TOKEN_IDENT)
            || is_matching_directive(ctx, 0, DIR_L)
            || is_matching_directive(ctx, 0, DIR_START);
}

enum parser_result parse_data_stmts(struct parser_context *ctx, struct ast_data_stmt **stmts, uint32_t *stmt_n) {

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
    if (follows_code_stmt(ctx)) {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Executable statement cannot occur in '.DATA' section.");
        goto _error;
    }

    *stmts = stmts_v.ptr;
    *stmt_n = stmts_v.length;

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




    if (is_matching_data_unit(ctx, 0, DATA_BYTE)) {
        stmt->kind = AST_DATA_STMT_BYTE;
        try_else(parse_byte_stmt(ctx, &stmt->byte_stmt), PARSER_OK, goto _error);
        _byte_stmt = true;
    } else if (is_matching_data_unit(ctx, 0, DATA_BYTES)) {
        stmt->kind = AST_DATA_STMT_BYTES;
        try_else(parse_bytes_stmt(ctx, &stmt->bytes_stmt), PARSER_OK, goto _error);
        _bytes_stmt = true;
    } else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        stmt->kind = AST_DATA_STMT_LABEL;
        try_else(parse_label_stmt(ctx, &stmt->label_stmt), PARSER_OK, goto _error);
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
    copy_terminal(ctx, &stmt->len);
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
    copy_terminal(ctx, &stmt->ident);
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
        copy_terminal(ctx, &init->number);

        next(ctx);
    } else if (is_matching_kind(ctx, 0, TOKEN_ASCII)) {
        init->kind = AST_INIT_ASCII;
        copy_terminal(ctx, &init->ascii);

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

    try_else(parse_numbers(ctx, &byte_init->numbers, &byte_init->num_n), PARSER_OK, goto _error);
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


enum parser_result parse_numbers(struct parser_context *ctx, struct ast_terminal **numbers, uint32_t *byte_n) {

    bool _numbers = false;



    struct vector numbers_v;
    try_else(vec_init(&numbers_v,4, sizeof(struct ast_terminal)), VEC_OK, goto _error);
    _numbers = true;

    struct ast_terminal number;

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        *numbers = numbers_v.ptr;
        *byte_n = numbers_v.length;
        return PARSER_OK;
    }

    copy_terminal(ctx, &number);

    try_else(vec_push(&numbers_v, &number), VEC_OK, goto _error);

    next(ctx);

    while (is_matching_punctuation(ctx, 0, PUNCT_COMMA)) {
        next(ctx);

        if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected a number after comma");
            goto _error;
        }
        copy_terminal(ctx, &number);

        try_else(vec_push(&numbers_v, &number), VEC_OK, goto _error);

        next(ctx);

    }

    *numbers = numbers_v.ptr;
    *byte_n = numbers_v.length;

    return PARSER_OK;
_error:
    if (_numbers) {
        vec_deinit(&numbers_v, NULL);
    }


    return PARSER_ERR;
}

enum parser_result parse_code_section(struct parser_context *ctx, struct ast_code_section *section) {
    bool _stmts = false;

    try_else(parse_code_dir(ctx), PARSER_OK, goto _error);

    try_else(parse_code_stmts(ctx, &section->code_stmts, &section->stmts_n), PARSER_OK, goto _error);
    _stmts = true;

    return PARSER_OK;

_error:

    if (_stmts) {
        for (uint32_t i = 0; i < section->stmts_n; i++) {
            ast_code_stmt_deinit(&section->code_stmts[i]);
        }
        free(section->code_stmts);
    }

    return PARSER_ERR;
}

enum parser_result parse_code_dir(struct parser_context *ctx) {
    if (!is_matching_directive(ctx, 0, DIR_CODE)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '.CODE'");
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



enum parser_result parse_code_stmts(struct parser_context *ctx, struct ast_code_stmt **stmts, uint32_t *stmt_n) {
    bool _stmts = false;
    bool _stmt = false;

    struct vector stmts_v;
    try_else(vec_init(&stmts_v, 5, sizeof(struct ast_code_stmt)), VEC_OK, goto _error);
    _stmts = true;

    struct ast_code_stmt stmt;

    while (follows_code_stmt(ctx)) {
        try_else(parse_code_stmt(ctx, &stmt), PARSER_OK, goto _error);
        _stmt = true;
        try_else(vec_push(&stmts_v, &stmt), VEC_OK, goto _error);
        _stmt = false;
    }

    if (follows_data_stmt(ctx)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Data statement cannot occur in '.CODE' section.");
        goto _error;
    }

    *stmts = stmts_v.ptr;
    *stmt_n = stmts_v.length;

    return PARSER_OK;

_error:
    if (_stmts) {
        vec_deinit(&stmts_v, &_ast_code_stmt_deinit);
    }

    if (_stmt) {
        ast_code_stmt_deinit(&stmt);
    }

    return PARSER_ERR;
}

enum parser_result parse_code_stmt(struct parser_context *ctx, struct ast_code_stmt *stmt) {
    bool _instr_stmt = false;


    if (is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        stmt->kind = AST_CODE_STMT_INSTRUCTION;
        try_else(parse_instruction_stmt(ctx, &stmt->instruction_stmt), PARSER_OK, goto _error);
        _instr_stmt = true;
    }
    else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        stmt->kind = AST_CODE_STMT_LABEL;
        try_else(parse_label_stmt(ctx, &stmt->label_stmt), PARSER_OK, goto _error);

    } else if (is_matching_directive(ctx, 0, DIR_L)) {
        stmt->kind = AST_CODE_STMT_LOC_LABEL;
        try_else(parse_loc_label_stmt(ctx, &stmt->loc_label_stmt), PARSER_OK, goto _error);

    } else if (is_matching_directive(ctx, 0, DIR_START)) {
        stmt->kind = AST_CODE_STMT_START;
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
    copy_terminal(ctx, &stmt->instr);

    next(ctx);

    if (is_matching_punctuation(ctx, 0, PUNCT_LPAR)) {
        try_else(parse_condition_code(ctx, &stmt->condition_code), PARSER_OK, goto _error);


    } else {
        stmt->condition_code.token = NULL;
    }

    try_else(parse_args(ctx, &stmt->args, &stmt->args_n), PARSER_OK, goto _error);
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
    copy_terminal(ctx, cond);
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

enum parser_result parse_args(struct parser_context *ctx, struct ast_arg **args, uint32_t *arg_n) {

    bool _args = false;


    struct vector args_v;

    try_else(vec_init(&args_v, 4, sizeof(struct ast_arg)), VEC_OK, goto _error);
    _args = true;

    struct ast_arg arg;

    if (!follows_arg(ctx)) {
        *args = args_v.ptr;
        *arg_n = args_v.length;
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
    *arg_n = args_v.length;

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
        copy_terminal(ctx, &arg->reg);
        next(ctx);

    } else if (is_matching_kind(ctx, 0, TOKEN_SYS_REG)) {
       arg->kind = AST_ARG_SYS_REG;
       copy_terminal(ctx, &arg->sys_reg);
       next(ctx);

    } else if (is_matching_kind(ctx, 0, TOKEN_PORT)) {
       arg->kind = AST_ARG_PORT;
       copy_terminal(ctx, &arg->port);
       next(ctx);

    } else if (is_matching_kind(ctx, 0, TOKEN_ADDR_REG)) {
       arg->kind = AST_ARG_ADDR_REG;
       copy_terminal(ctx, &arg->addr_reg);
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
    copy_terminal(ctx, immediate);

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
    copy_terminal(ctx, &label->ident);

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
    copy_terminal(ctx, &label->ident);

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
    copy_terminal(ctx, dir);
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
    copy_terminal(ctx, dist);

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
    copy_terminal(ctx, &stmt->ident);

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




enum parser_result parse_head_section(struct parser_context *ctx, struct ast_head_section *section) {

    try_else(parse_head_dir(ctx), PARSER_OK, goto _error);

    try_else(parse_head_stmts(ctx, &section->stmts, &section->stmt_n), PARSER_OK, goto _error);

    return PARSER_OK;

_error:
    return PARSER_ERR;
}

enum parser_result parse_head_dir(struct parser_context *ctx) {

    if (!is_matching_directive(ctx, 0, DIR_HEAD)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected '.HEAD' directive.");
        goto _error;
    }
    next(ctx);

    if (!is_matching_punctuation(ctx, 0, PUNCT_SEMICOLON)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected : after '.HEAD' directive.");
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


static bool follows_head_stmt(struct parser_context *ctx) {

    return is_matching_directive(ctx, 0, DIR_GLOB)
        || is_matching_directive(ctx, 0, DIR_EXTERN);
}

enum parser_result parse_head_stmts(struct parser_context *ctx, struct ast_head_stmt **stmts, uint32_t *stmt_n) {
    bool _stmtsv = false;

    struct vector stmtsv;
    try_else(vec_init(&stmtsv, 10, sizeof(struct ast_head_stmt)), VEC_OK, goto _error);
    _stmtsv = true;

    while (follows_head_stmt(ctx)) {
        struct ast_head_stmt stmt;
        try_else(parse_head_stmt(ctx, &stmt), PARSER_OK, goto _error);
        try_else(vec_push(&stmtsv, &stmt), VEC_OK, goto _error);
    }

    *stmts = stmtsv.ptr;
    *stmt_n = stmtsv.length;


    return PARSER_OK;
_error:
    if (_stmtsv) {
        vec_deinit(&stmtsv, NULL);
    }
    return PARSER_ERR;
}

enum parser_result parse_head_stmt(struct parser_context *ctx, struct ast_head_stmt *stmt) {
    if (is_matching_directive(ctx, 0, DIR_GLOB)) {
        stmt->kind = AST_HEAD_STMT_GLOB;
        try_else(parse_glob_stmt(ctx, &stmt->glob_stmt), PARSER_OK, goto _error);


    } else if (is_matching_directive(ctx, 0, DIR_EXTERN)) {
        stmt->kind = AST_HEAD_STMT_EXTERN;
        try_else(parse_extern_stmt(ctx, &stmt->extern_stmt), PARSER_OK, goto _error);

    } else {
        snprintf(ctx->error_msg,  ERR_MSG_LEN, "Expected statement '.glob' or '.extern'");
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
    return PARSER_ERR;
}

enum parser_result parse_glob_stmt(struct parser_context *ctx, struct ast_glob_stmt *stmt) {
    if (!is_matching_directive(ctx, 0, DIR_GLOB)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    next(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    copy_terminal(ctx, &stmt->ident);

    next(ctx);

    return PARSER_OK;




_error:
    return PARSER_ERR;
}

enum parser_result parse_extern_stmt(struct parser_context *ctx, struct ast_extern_stmt *stmt) {
    if (!is_matching_directive(ctx, 0, DIR_EXTERN)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    next(ctx);

    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Expected label identifier");
        goto _error;
    }
    copy_terminal(ctx, &stmt->ident);

    next(ctx);


    return PARSER_OK;



_error:
    return PARSER_ERR;
}
