
#ifndef __AST_HEADER__
#define __AST_HEADER__


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "shared/token.h"


struct ast_terminal {
    struct token *token;
};


struct ast_file {
    struct ast_section *sections;
    uint32_t sec_n;
    const char *filename;
};


enum ast_section_kind {
    AST_CODE_SECTION,
    AST_DATA_SECTION,
    AST_HEAD_SECTION
};

struct ast_data_section {
    struct ast_data_stmt *data_stmts;
    uint32_t stmts_n;
};

struct ast_code_section {
    struct ast_code_stmt *code_stmts;
    uint32_t stmts_n;
};

enum ast_head_stmt_kind {
    AST_HEAD_STMT_GLOB,
    AST_HEAD_STMT_EXTERN
};

struct ast_glob_stmt {
    struct ast_terminal ident;
};

struct ast_extern_stmt {
    struct ast_terminal ident;
};

struct ast_head_stmt {
    uint32_t line;
    enum ast_head_stmt_kind kind;
    union {
        struct ast_glob_stmt glob_stmt;
        struct ast_extern_stmt extern_stmt;
    };
};

struct ast_head_section {
    struct ast_head_stmt *stmts;
    uint32_t stmt_n;
};


struct ast_section {
    enum ast_section_kind kind;
    union {
        struct ast_data_section data_section;
        struct ast_code_section code_section;
        struct ast_head_section head_section;
    };
};



enum ast_initializer_kind {
    AST_INIT_NUM,
    AST_INIT_ASCII,
    AST_INIT_BYTE_INIT,
    AST_INIT_NONE
};

struct ast_byte_init {
    struct ast_terminal *numbers;
    uint32_t num_n;
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
    struct ast_initializer init;
};

struct ast_bytes_stmt {

    struct ast_initializer init;
    struct ast_terminal len;
};

struct ast_label_stmt {

    struct ast_terminal ident;

};

enum ast_data_stmt_kind {
    AST_DATA_STMT_BYTE,
    AST_DATA_STMT_BYTES,
    AST_DATA_STMT_LABEL
};

struct ast_data_stmt {
    uint32_t line;
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
    uint32_t args_n;
};


struct ast_loc_label_stmt {
    union {
        struct ast_terminal ident;
        uint32_t offset;
    };
};

enum ast_code_stmt_kind {
    AST_CODE_STMT_INSTRUCTION,
    AST_CODE_STMT_LABEL,
    AST_CODE_STMT_LOC_LABEL,
    AST_CODE_STMT_START
};

struct ast_code_stmt {
    uint32_t line;
    enum ast_code_stmt_kind kind;
    union {
        struct ast_instruction_stmt instruction_stmt;
        struct ast_label_stmt label_stmt;
        struct ast_loc_label_stmt loc_label_stmt;
    };
};








struct ast_loc_label {
    struct ast_terminal dir;
    struct ast_terminal ident;

};


enum ast_arg_kind {
    AST_ARG_REG,
    AST_ARG_SYS_REG,
    AST_ARG_ADDR_REG,
    AST_ARG_PORT,
    AST_ARG_IMMEDIATE,
    AST_ARG_LABEL,
    AST_ARG_LOC_LABEL,
};

struct ast_label {

    struct ast_terminal ident;


};

struct ast_arg {
    enum ast_arg_kind kind;
    union {
        struct ast_terminal reg;
        struct ast_terminal sys_reg;
        struct ast_terminal addr_reg;
        struct ast_terminal port;
        struct ast_terminal immediate;
        struct ast_label label;
        struct ast_loc_label loc_label;
    };
};







void ast_file_deinit(struct ast_file *node);
void ast_data_section_deinit(struct ast_data_section *node);
void ast_code_section_deinit(struct ast_code_section *node);
void ast_section_deinit(struct ast_section *node);
void ast_initializer_deinit(struct ast_initializer *node);
void ast_byte_stmt_deinit(struct ast_byte_stmt *node);
void ast_bytes_stmt_deinit(struct ast_bytes_stmt *node);
void ast_data_stmt_deinit(struct ast_data_stmt *node);
void ast_instruction_stmt_deinit(struct ast_instruction_stmt *node);
void ast_code_stmt_deinit(struct ast_code_stmt *node);
void ast_head_section_deinit(struct ast_head_section *node);




void _ast_file_deinit(void *node);
void _ast_data_section_deinit(void *node);
void _ast_code_section_deinit(void *node);
void _ast_section_deinit(void *node);
void _ast_initializer_deinit(void *node);
void _ast_byte_stmt_deinit(void *node);
void _ast_bytes_stmt_deinit(void *node);
void _ast_data_stmt_deinit(void *node);
void _ast_instruction_stmt_deinit(void *node);
void _ast_code_stmt_deinit(void *node);
void _ast_head_section_deinit(void *node);







void print_ast_file(struct ast_file *file);
#endif
