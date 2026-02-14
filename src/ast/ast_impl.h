
#ifndef __AST_IMPL_HEADER__
#define __AST_IMPL_HEADER__


#include "ast.h"



void build_ast_file(const struct cst_node *cst, struct ast_file *ast);

void build_ast_exec_stmt(const struct cst_node *cst, struct ast_exec_stmt *ast);

void build_ast_instr_stmt(const struct cst_node *cst, struct ast_instr_stmt *ast);

void build_ast_macro_stmt(const struct cst_node *cst, struct ast_macro_stmt *ast);

void build_ast_label_stmt(const struct cst_node *cst, struct ast_label_stmt *ast);

void build_ast_l_label_stmt(const struct cst_node *cst, struct ast_l_label_stmt *ast);

void build_ast_arg(const struct cst_node *cst, struct ast_arg *ast);

void build_ast_l_label(const struct cst_node *cst, struct ast_l_label *ast);

void build_ast_data_stmt(const struct cst_node *cst, struct ast_data_stmt *ast);

void build_ast_byte_stmt(const struct cst_node *cst, struct ast_byte_stmt *ast);

void build_ast_bytes_stmt(const struct cst_node *cst, struct ast_bytes_stmt *ast);

void build_ast_init(const struct cst_node *cst, struct ast_init *ast);

enum ast_reg get_ast_reg(const char *str);

enum ast_addr_reg get_ast_addr_reg(const char *str);

enum ast_sys_reg get_ast_sys_reg(const char *str);

enum ast_port get_ast_port(const char *str);

enum ast_condition_code get_ast_cond_code(const char *str);




#endif
