
#ifndef __AST_HEADER__
#define __AST_HEADER__


#include <stdbool.h>
#include <stdint.h>

#include "shared/token.h"




enum ast_stmt_kind {
    AST_CODE_STMT,
    AST_DATA_STMT,
    AST_IMPORT_STMT,
    AST_EXPORT_STMT,
    AST_REP_STMT,
    AST_ENDREP_STMT,
    AST_LABEL_STMT,
    AST_LOC_LABEL_STMT,
    AST_INSTRUCTION_STMT,
    AST_BYTE_STMT,
    AST_WORD_STMT,
    AST_DWORD_STMT,
    AST_QWORD_STMT,
    AST_ASCII_STMT,
    AST_ASCIZ_STMT
};


enum ast_arg_kind {
    AST_REG_ARG,
    AST_SYS_REG_ARG,
    AST_ADDR_REG_ARG,
    AST_PORT_ARG,
    AST_IMMEDIATE_ARG,
    AST_LABEL_ARG,
    AST_F_LOC_LABEL_ARG,
    AST_B_LOC_LABEL_ARG
};

struct ast_arg {
    enum ast_arg_kind kind;

    union {
        struct token *reg;
        struct token *sys_reg;
        struct token *port;
        struct token *addr_reg;
        struct token *immediate;
        struct token *label;
        struct token *loc_label;
    };

};

struct ast_instruction_stmt {
    struct token *mnemonic;
    struct token *cond_code;
    struct ast_arg args[3];
};

struct ast_stmt {
    enum ast_stmt_kind kind;

    union {
        struct token *code_name;
        struct token *data_name;
        struct token *import_label;
        struct token *export_label;
        struct token *rep_count;
        struct token *label;
        struct token *loc_label;
        struct ast_instruction_stmt *instruction;
        struct token *initializer;
    };
};


struct ast_file {
    struct ast_stmt *stmts;
    uint32_t stmt_n;
};



void ast_file_deinit(struct ast_file *file);



#endif
