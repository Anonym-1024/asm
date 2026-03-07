
#ifndef __AST_HEADER__
#define __AST_HEADER__


#include <stdlib.h>
#include <stdbool.h>




struct ast_terminal {
    struct token *token;
};


struct ast_file {
    struct ast_section *sections;
    size_t sec_n;
};


enum ast_section_kind {
    AST_EXEC_SECTION,
    AST_DATA_SECTION
};

struct ast_data_section {
    struct ast_data_stmt *data_stmts;
    size_t stmts_c;
};

struct ast_exec_section {
    struct ast_exec_stmt *exec_stmts;
    size_t stmts_c;
};

struct ast_section {
    enum ast_section_kind kind;
    union {
        struct ast_data_section data_section;
        struct ast_exec_section exec_section;
    };
};



enum ast_initializer_kind {
    AST_INIT_NUM,
    AST_INIT_ASCII,
    AST_INIT_BYTE_INIT
};

struct ast_byte_init {
    struct ast_terminal *numbers;
    size_t num_c;
};

struct ast_initializer {
    enum ast_initializer_kind kind;

    union {
        struct ast_terminal number;
        struct ast_terminal ascii;
        struct ast_byte_init byte_init;
    };
};

struct ast_byte_stmt {
    bool is_initialized;
    struct ast_initializer init;
};

struct ast_bytes_stmt {
    struct ast_initializer init;
    struct ast_terminal len;
    bool is_initialized;
};

struct ast_label_stmt {
    struct ast_terminal ident;
};

enum ast_data_stmt_kind {
    AST_DATA_STMT_BYTE_STMT,
    AST_DATA_STMT_BYTES_STMT,
    AST_DATA_STMT_LABEL_STMT
};

struct ast_data_stmt {
    enum ast_data_stmt_kind kind;
    union {
        struct ast_byte_stmt byte_stmt;
        struct ast_bytes_stmt bytes_stmt;
        struct ast_label_stmt label_stmt;
    };
};




struct ast_instruction_stmt {
    struct ast_terminal instr;
    struct ast_terminal condition_code;
    struct ast_arg *args;
    size_t args_c;
};

struct ast_macro_stmt {
    struct ast_terminal instr;
    struct ast_terminal condition_code;
    struct ast_arg *args;
    size_t args_c;
};

struct ast_loc_label_stmt {
    struct ast_terminal ident;
};

enum ast_exec_stmt_kind {
    AST_EXEC_STMT_INSTRUCTION_STMT,
    AST_EXEC_STMT_MACRO_STMT,
    AST_EXEC_STMT_LABEL_STMT,
    AST_EXEC_STMT_LOC_LABEL_STMT,
    AST_EXEC_STMT_START_STMT
};

struct ast_exec_stmt {
    enum ast_exec_stmt_kind kind;
    union {
        struct ast_instruction_stmt instruction_stmt;
        struct ast_macro_stmt macro_stmt;
        struct ast_label_stmt label_stmt;
        struct ast_loc_label_stmt loc_label_stmt;
    };
};











struct ast_loc_label {
    bool has_dist;
    struct ast_terminal dir;
    struct ast_terminal dist;
    struct ast_terminal ident;
};

enum ast_arg_kind {
    AST_ARG_REG,
    AST_ARG_SYS_RES,
    AST_ARG_ADDR_REG,
    AST_ARG_PORT,
    AST_ARG_IMMEDIATE,
    AST_ARG_LABEL,
    AST_ARG_LOC_LABEL
};

struct ast_arg {
    enum ast_arg_kind kind;
    union {
        struct ast_terminal reg;
        struct ast_terminal sys_reg;
        struct ast_terminal addr_reg;
        struct ast_terminal port;
        struct ast_terminal immediate;
        struct ast_terminal label;
        struct ast_loc_label loc_label;
    };
};



void ast_file_deinit(struct ast_file *node);
void ast_data_section_deinit(struct ast_data_section *node);
void ast_exec_section_deinit(struct ast_exec_section *node);
void ast_section_deinit(struct ast_section *node);
void ast_initializer_deinit(struct ast_initializer *node);
void ast_byte_stmt_deinit(struct ast_byte_stmt *node);
void ast_bytes_stmt_deinit(struct ast_bytes_stmt *node);
void ast_data_stmt_deinit(struct ast_data_stmt *node);
void ast_instruction_stmt_deinit(struct ast_instruction_stmt *node);
void ast_macro_stmt_deinit(struct ast_macro_stmt *node);
void ast_exec_stmt_deinit(struct ast_exec_stmt *node);




void _ast_file_deinit(void *node);
void _ast_data_section_deinit(void *node);
void _ast_exec_section_deinit(void *node);
void _ast_section_deinit(void *node);
void _ast_initializer_deinit(void *node);
void _ast_byte_stmt_deinit(void *node);
void _ast_bytes_stmt_deinit(void *node);
void _ast_data_stmt_deinit(void *node);
void _ast_instruction_stmt_deinit(void *node);
void _ast_macro_stmt_deinit(void *node);
void _ast_exec_stmt_deinit(void *node);



/*
void null_ast_terminal(struct ast_terminal *node);
void null_ast_file(struct ast_file *node);
void null_ast_data_section(struct ast_data_section *node);
void null_ast_exec_section(struct ast_exec_section *node);
void null_ast_section(struct ast_section *node);
void null_ast_byte_initializer(struct ast_byte_initializer *node);
void null_ast_initializer(struct ast_initializer *node);
void null_ast_byte_stmt(struct ast_byte_stmt *node);
void null_ast_bytes_stmt(struct ast_bytes_stmt *node);
void null_ast_label_stmt(struct ast_label_stmt *node);
void null_ast_data_stmt(struct ast_data_stmt *node);
void null_ast_instruction_stmt(struct ast_instruction_stmt *node);
void null_ast_macro_stmt(struct ast_macro_stmt *node);
void null_ast_loc_label_stmt(struct ast_loc_label_stmt *node);
void null_ast_exec_stmt(struct ast_exec_stmt *node);
void null_ast_loc_label(struct ast_loc_label *node);
void null_ast_arg(struct ast_arg *node);

*/


void print_ast_file(struct ast_file *file);
#endif
