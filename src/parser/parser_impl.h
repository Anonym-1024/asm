
#ifndef __PARSER_IMPL_HEADER__
#define __PARSER_IMPL_HEADER__


#include "error/compiler_error.h"
#include "shared/token.h"
#include "shared/ast.h"
#include <stdint.h>

struct parser_context {
    struct token *in;
    uint32_t n;
    uint32_t index;

    uint32_t line;
    uint16_t col;
    char error_msg[ERR_MSG_LEN];
};


enum parser_result {
    PARSER_OK,
    PARSER_ERR
};


enum parser_result parse_file(struct parser_context *ctx, struct ast_file *file);

enum parser_result parse_sections(struct parser_context *ctx, struct ast_section **sections, uint32_t *sec_c);

enum parser_result parse_section(struct parser_context *ctx, struct ast_section *section);

enum parser_result parse_data_section(struct parser_context *ctx, struct ast_data_section *section);

enum parser_result parse_data_dir(struct parser_context *ctx);

enum parser_result parse_data_stmts(struct parser_context *ctx, struct ast_data_stmt **stmts, uint32_t *stmt_c);

enum parser_result parse_data_stmt(struct parser_context *ctx, struct ast_data_stmt *stmt);

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct ast_byte_stmt *stmt);

enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct ast_bytes_stmt *stmt);

enum parser_result parse_label_stmt(struct parser_context *ctx, struct ast_label_stmt *stmt);

enum parser_result parse_initializer(struct parser_context *ctx, struct ast_initializer *init);

enum parser_result parse_byte_initializer(struct parser_context *ctx, struct ast_byte_init *byte_init);

enum parser_result parse_numbers(struct parser_context *ctx, struct ast_terminal **numbers, uint32_t *byte_c);

enum parser_result parse_code_section(struct parser_context *ctx, struct ast_code_section *section);

enum parser_result parse_code_dir(struct parser_context *ctx);

enum parser_result parse_code_stmts(struct parser_context *ctx, struct ast_code_stmt **stmts, uint32_t *stmt_c);

enum parser_result parse_code_stmt(struct parser_context *ctx, struct ast_code_stmt *stmt);

enum parser_result parse_start_stmt(struct parser_context *ctx);

enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct ast_instruction_stmt *stmt);

enum parser_result parse_condition_code(struct parser_context *ctx, struct ast_terminal *cond);

enum parser_result parse_args(struct parser_context *ctx, struct ast_arg **args, uint32_t *arg_c);

enum parser_result parse_arg(struct parser_context *ctx, struct ast_arg *arg);

enum parser_result parse_immediate(struct parser_context *ctx, struct ast_terminal *immediate);

enum parser_result parse_label(struct parser_context *ctx, struct ast_label *label);

enum parser_result parse_loc_label(struct parser_context *ctx, struct ast_loc_label *label);

enum parser_result parse_direction_dir(struct parser_context *ctx, struct ast_terminal *dir);

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct ast_terminal *dist);

enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct ast_loc_label_stmt *stmt);

enum parser_result parse_head_section(struct parser_context *ctx, struct ast_head_section *section);

enum parser_result parse_head_dir(struct parser_context *ctx);

enum parser_result parse_head_stmts(struct parser_context *ctx, struct ast_head_stmt **stmts, uint32_t stmt_c);

enum parser_result parse_head_stmt(struct parser_context *ctx, struct ast_head_stmt *stmt);

enum parser_result parse_glob_stmt(struct parser_context *ctx, struct ast_glob_stmt *stmt);

enum parser_result parse_extern_stmt(struct parser_context *ctx, struct ast_extern_stmt *stmt);



#endif
