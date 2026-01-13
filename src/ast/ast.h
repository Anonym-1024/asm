
#ifndef __AST_HEADER__ 
#define __AST_HEADER__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <stdint.h>

#include "libs/vector/vector.h"
#include "parser/parser.h"

typedef int64_t num_lit_t; 


struct ast_label_stmt {
    char *label;
};


enum ast_init_kind {
    INIT_NUM,
    INIT_ASCII,
    INIT_BYTES,
    INIT_NONE
};

struct ast_init {
    enum ast_init_kind kind;
    union {
        num_lit_t num;
        char *ascii;
        uint8_t *bytes;
    };
};


struct ast_bytes_stmt {
    num_lit_t length;
    struct ast_init init;
};

struct ast_byte_stmt {
    struct ast_init init;
};


enum ast_data_stmt_kind {
    D_STMT_BYTE,
    D_STMT_BYTES,
    D_STMT_LABEL
};

struct ast_data_stmt {
    enum ast_data_stmt_kind kind;
    union {
        struct ast_byte_stmt byte_stmt;
        struct ast_bytes_stmt bytes_stmt;
        struct ast_label_stmt label_stmt;
    };
};



enum ast_arg_kind {
    ARG_REG,
    ARG_SYS_REG,
    ARG_PORT,
    ARG_ADDR_REG,
    ARG_IMM,
    ARG_LABEL,
    ARG_L_LABEL
};


enum ast_reg {
    REG_0 = 0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7,
    REG_8,
    REG_9,
    REG_10,
    REG_11,
    REG_12,
    REG_13,
    REG_14,
    REG_15
};


enum ast_addr_reg {
    REG_0_A = 0,
    REG_1_A,
    REG_3_A,
    REG_4_A,
    REG_5_A,
    REG_6_A,
    REG_7_A,
    REG_8_A,
    REG_9_A,
    REG_10_A,
    REG_11_A,
    REG_12_A,
    REG_13_A,
    REG_14_A,
    REG_15_A
};

enum ast_port {
    PORT_0 = 0,
    PORT_1,
    PORT_2,
    PORT_3,
    PORT_4,
    PORT_5,
    PORT_6,
    PORT_7
};

enum ast_sys_reg {
    PC_B0,
    PC_B1,
    PSR,
    INTR,
    PDBR_B0,
    PDBR_B1
};


struct ast_l_label {
    char *label;
    bool back;
    num_lit_t dist;
};

struct ast_arg {
    enum ast_arg_kind kind;
    union {
        enum ast_reg reg;
        enum ast_addr_reg addr_reg;
        enum ast_sys_reg sys_reg;
        enum ast_port port;
        num_lit_t imm;
        char *label;
        struct ast_l_label l_label;
    };
};

enum ast_condition_code {
    CC_AL,
    /// TODO:
};

struct ast_instr_stmt {
    char *mnemonic;
    enum ast_condition_code cond;
    struct vector args;
};

struct ast_macro_stmt {
    char *mnemonic;
    enum ast_condition_code cond;
    struct vector args;
};

struct ast_l_label_stmt {
    char *label;
};




enum ast_exec_stmt_kind {
    E_STMT_INSTR,
    E_STMT_MACRO,
    E_STMT_LABEL,
    E_STMT_L_LABEL
};

struct ast_exec_stmt {
    enum ast_exec_stmt_kind kind;
    union {
        struct ast_instr_stmt instr_stmt;
        struct ast_macro_stmt macro_stmt;
        struct ast_label_stmt label_stmt;
        struct ast_l_label_stmt l_label_stmt;
    };
};


struct ast_file {
    struct vector data_stmts;
    struct vector exec_stmts;
};


void construct_ast(const struct cst_node *cnode, struct ast_file *ast);




#endif
