
#ifndef __PARSER_HEADER__ 
#define __PARSER_HEADER__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



#include "parser_error.h"
#include "libs/vector/vector.h"
#include "lexer/lexer.h"



enum cst_node_kind {
    CST_TERMINAL,
    CST_FILE,
    CST_SECTIONS,
    CST_SECTION,
    CST_DATA_SECTION,
    CST_DATA_DIR,
    CST_DATA_STMTS,
    CST_DATA_STMT,
    CST_BYTE_STMT,
    CST_BYTES_STMT,
    CST_LABEL_STMT,
    CST_INITIALIZER,
    CST_BYTE_INITIALIZER,
    CST_NUMBERS,
    CST_EXEC_SECTION,
    CST_EXEC_DIR,
    CST_EXEC_STMTS,
    CST_EXEC_STMT,
    CST_INSTRUCTION_STMT,
    CST_CONDITION_CODE,
    CST_ARGS,
    CST_ARG,
    CST_IMMEDIATE,
    CST_LABEL,
    CST_LOC_LABEL,
    CST_DIRECTION_DIR,
    CST_LOC_LABEL_DIST,
    CST_MACRO_STMT,
    CST_LOC_LABEL_STMT

};



struct cst_node {
    enum cst_node_kind kind;

    struct token terminal;
    struct vector children;
};


void print_cst_node(FILE *f, struct cst_node *n, int indent);


enum parser_result {
    PARSER_OK,
    PARSER_ERR,
};

enum parser_result parse(const struct token *in, size_t n, struct cst_node *out, struct parser_error *error);


#endif
