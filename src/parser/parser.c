
#include "parser.h"
#include "parser_impl.h"


#include "libs/utilities/utilities.h"





static const char *cst_node_kind_to_string(enum cst_node_kind kind) {
    switch (kind) {
        case CST_TERMINAL: return "CST_TERMINAL";
        case CST_FILE: return "CST_FILE";
        case CST_SECTIONS: return "CST_SECTIONS";
        case CST_SECTION: return "CST_SECTION";
        case CST_DATA_SECTION: return "CST_DATA_SECTION";
        case CST_DATA_DIR: return "CST_DATA_DIR";
        case CST_DATA_STMTS: return "CST_DATA_STMTS";
        case CST_DATA_STMT: return "CST_DATA_STMT";
        case CST_BYTE_STMT: return "CST_BYTE_STMT";
        case CST_BYTES_STMT: return "CST_BYTES_STMT";
        case CST_LABEL_STMT: return "CST_LABEL_STMT";
        case CST_INITIALIZER: return "CST_INITIALIZER";
        case CST_BYTE_INITIALIZER: return "CST_BYTE_INITIALIZER";
        case CST_NUMBERS: return "CST_NUMBERS";
        case CST_EXEC_SECTION: return "CST_EXEC_SECTION";
        case CST_EXEC_DIR: return "CST_EXEC_DIR";
        case CST_EXEC_STMTS: return "CST_EXEC_STMTS";
        case CST_EXEC_STMT: return "CST_EXEC_STMT";
        case CST_INSTRUCTION_STMT: return "CST_INSTRUCTION_STMT";
        case CST_CONDITION_CODE: return "CST_CONDITION_CODE";
        case CST_ARGS: return "CST_ARGS";
        case CST_ARG: return "CST_ARG";
        case CST_IMMEDIATE: return "CST_IMMEDIATE";
        case CST_LABEL: return "CST_LABEL";
        case CST_LOC_LABEL: return "CST_LOC_LABEL";
        case CST_DIRECTION_DIR: return "CST_DIRECTION_DIR";
        case CST_LOC_LABEL_DIST: return "CST_LOC_LABEL_DIST";
        case CST_MACRO_STMT: return "CST_MACRO_STMT";
        case CST_LOC_LABEL_STMT: return "CST_LOC_LABEL_STMT";
        default: return "UNKNOWN";
    }
}



void print_cst_node(FILE *f, struct cst_node *n, int indent) {
    if (!n) return;

    for (int i = 0; i < indent; i++)
        fputs("|  ", f);

    fprintf(f, "%s", cst_node_kind_to_string(n->kind));

    if (n->kind == CST_TERMINAL) {
        /* Adjust field name if your token differs */
        fprintf(f, "'%s'", n->terminal.lexeme);
        fputc('\n', f);
    } else {

    
        fputc('\n', f);
        for (size_t i = 0; i < n->children.length; i++) {
            print_cst_node(
                f,
                vec_get_ptr_t(&n->children, i, struct cst_node),
                indent + 1
            );
        }
        
    }
}



static size_t current_line(struct parser_context *ctx) {
    if (ctx->index >= ctx->n) {
        return 0;
    }
    return ctx->in[ctx->index].line;
}


static size_t current_col(struct parser_context *ctx) {
    if (ctx->index >= ctx->n) {
        return 0;
    }
    return ctx->in[ctx->index].col;
}


enum parser_result parse(const struct token *in, size_t n, struct cst_node *out, struct parser_error *error) {

    struct parser_context ctx = {
        .in = in,
        .index = 0,
        .n = n,

        .line = 1,
        .col = 1,
        .error = NULL
    };

    struct cst_node node;
    enum parser_result res = parse_file(&ctx, &node);

    if (res == PARSER_ERR) {
        error->line = current_line(&ctx);
        error->col = current_col(&ctx);
        error->msg = ctx.error;

        return PARSER_ERR;
    } 


    *out = node;
    return PARSER_OK;
}








