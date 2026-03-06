
#ifndef __PARSER_IMPL_HEADER__
#define __PARSER_IMPL_HEADER__



#include "lexer/lexer.h"
#include "ast.h"

struct parser_context {
    struct token *in;
    size_t n;
    size_t index;

    size_t line;
    size_t col;
    char *error_msg;
};


enum parser_result {
    PARSER_OK,
    PARSER_ERR
};


enum parser_result parse_file(struct parser_context *ctx, struct ast_file *file);

enum parser_result parse_sections(struct parser_context *ctx, struct ast_section **sections, size_t *sec_c);

enum parser_result parse_section(struct parser_context *ctx, struct ast_section *section);

enum parser_result parse_data_section(struct parser_context *ctx, struct ast_data_section *section);

enum parser_result parse_data_dir(struct parser_context *ctx);

enum parser_result parse_data_stmts(struct parser_context *ctx, struct ast_data_stmt **stmts, size_t *stmt_c);

enum parser_result parse_data_stmt(struct parser_context *ctx, struct ast_data_stmt *stmt);

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct ast_byte_stmt *stmt);

enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct ast_bytes_stmt *stmt);

enum parser_result parse_label_stmt(struct parser_context *ctx, struct ast_label_stmt *stmt);

enum parser_result parse_initializer(struct parser_context *ctx, struct ast_initializer *init);

enum parser_result parse_byte_initializer(struct parser_context *ctx, struct ast_byte_init *byte_init);

enum parser_result parse_numbers(struct parser_context *ctx, struct ast_terminal **numbers, size_t *byte_c);

enum parser_result parse_exec_section(struct parser_context *ctx, struct ast_exec_section *section);

enum parser_result parse_exec_dir(struct parser_context *ctx);

enum parser_result parse_exec_stmts(struct parser_context *ctx, struct ast_exec_stmt **stmts, size_t *stmt_c);

enum parser_result parse_exec_stmt(struct parser_context *ctx, struct ast_exec_stmt *stmt);

enum parser_result parse_start_stmt(struct parser_context *ctx);

enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct ast_instruction_stmt *stmt);

enum parser_result parse_condition_code(struct parser_context *ctx, struct ast_terminal *cond);

enum parser_result parse_args(struct parser_context *ctx, struct ast_arg **args, size_t *arg_c);

enum parser_result parse_arg(struct parser_context *ctx, struct ast_arg *arg);

enum parser_result parse_immediate(struct parser_context *ctx, struct ast_terminal *immediate);

enum parser_result parse_label(struct parser_context *ctx, struct ast_terminal *label);

enum parser_result parse_loc_label(struct parser_context *ctx, struct ast_loc_label *label);

enum parser_result parse_direction_dir(struct parser_context *ctx, struct ast_terminal *dir);

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct ast_terminal *dist);

enum parser_result parse_macro_stmt(struct parser_context *ctx, struct ast_macro_stmt *stmt);

enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct ast_loc_label_stmt *stmt);





#endif
