
#ifndef __PARSER_HELPERS_HEADER
#define __PARSER_HELPERS_HEADER

#include "parser.h"



struct parser_context {
    const struct token *in;
    size_t n;
    size_t index;

    size_t line;
    size_t col;

    char *error;
};


void cst_node_deinit(void *node);

enum parser_result parse_file(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_sections(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_section(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_data_section(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_data_dir(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_data_stmts(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_data_stmt(struct parser_context *ctx, struct cst_node *node); 

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_label_stmt(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_initializer(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_byte_initializer(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_numbers(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_exec_section(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_exec_dir(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_exec_stmts(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_exec_stmt(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_condition_code(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_args(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_arg(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_immediate(struct parser_context *ctx, struct cst_node *node);

enum parser_result parse_label(struct parser_context *ctx, struct cst_node *node); 

enum parser_result parse_loc_label(struct parser_context *ctx, struct cst_node *node) ;

enum parser_result parse_direction_dir(struct parser_context *ctx, struct cst_node *node) ;

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct cst_node *node) ;

enum parser_result parse_macro_stmt(struct parser_context *ctx, struct cst_node *node) ;

enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct cst_node *node);

#endif
