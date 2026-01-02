
#include "parser.h"
#include "parser_helpers.h"


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
        fprintf(f, "' %s'", n->terminal.lexeme);
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








static bool is_matching_kind(struct parser_context *ctx, int offset, enum token_kind kind) {
    size_t index = ctx->index + offset;
    if (index >= ctx->n) {
        return false;
    }

    return (ctx->in[index].kind == kind);
    
}

static bool is_matching_lexeme(struct parser_context *ctx, int offset, const char *lexeme) {
    size_t index = ctx->index + offset;
    if (index >= ctx->n) {
        return false;
    }

    return strcmp(ctx->in[index].lexeme, lexeme) == 0;
}

static void pop_token(struct parser_context *ctx, struct token *t) {
    *t = ctx->in[ctx->index];
    t->lexeme = mallocs(strlen(t->lexeme) * sizeof(char) + 1);
    strcpy(t->lexeme, ctx->in[ctx->index].lexeme);
    ctx->index += 1;
}

static const char *current_token_lexeme(struct parser_context *ctx) {
    if (ctx->index < ctx->n) {
        return ctx->in[ctx->index].lexeme;
    }
    return "End of file.";
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

static bool is_line_terminated(struct parser_context *ctx) {
    if (is_matching_lexeme(ctx, 0, "\n")) {
        do {
            ctx->index += 1;
        } while (is_matching_lexeme(ctx, 0, "\n"));
        return true;
    } else if (is_matching_kind(ctx, 0, TOKEN_EOF)) {
        return true;
    }
    return false;
}





static struct cst_node make_terminal_node(struct parser_context *ctx) {
    struct cst_node node;
    node.kind = CST_TERMINAL;
    pop_token(ctx, &node.terminal);
    return node;
}



void cst_node_deinit(void *node) {
    struct cst_node *n = (struct cst_node*)node;

    if (n->kind == CST_TERMINAL) {
        free(n->terminal.lexeme);
    } else {
        vec_deinit(&n->children, &cst_node_deinit);
    }
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








enum parser_result parse_file(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    while (is_matching_lexeme(ctx, 0, "\n")) {
        ctx->index += 1;
    }

    struct cst_node sections_node;
    res = parse_sections(ctx, &sections_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &sections_node);


    while (is_matching_lexeme(ctx, 0, "\n")) {
        ctx->index += 1;
    }

    if (!is_matching_kind(ctx, 0, TOKEN_EOF)) {
        asprintfs(&ctx->error, "Expected end of file ");
        goto _error;
    }



    node->children = children;
    node->kind = CST_FILE;
    return PARSER_OK;


    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


bool follows_section(struct parser_context *ctx) {
    return is_matching_lexeme(ctx, 0, ".DATA") || is_matching_lexeme(ctx, 0, ".EXEC");
}

enum parser_result parse_sections(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node section_node;
    while (follows_section(ctx)) {
        res = parse_section(ctx, &section_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &section_node);
    }
    
    
    node->children = children;
    node->kind = CST_SECTIONS;
    return PARSER_OK;


    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_section(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY


    struct cst_node child_node;
    if (is_matching_lexeme(ctx, 0, ".DATA")) {
        res = parse_data_section(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        

    } else {
        res = parse_exec_section(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        
    }
    vec_pushs(&children, &child_node);
    
    node->children = children;
    node->kind = CST_SECTION;
    return PARSER_OK;



    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_data_section(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node data_dir_node;
    res = parse_data_dir(ctx, &data_dir_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &data_dir_node);


    struct cst_node data_stmts_node;
    res = parse_data_stmts(ctx, &data_stmts_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &data_stmts_node);




    node->children = children;
    node->kind = CST_DATA_SECTION;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_data_dir(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    if (!is_matching_lexeme(ctx, 0, ".DATA")) {
        asprintfs(&ctx->error, "Expected '.DATA', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node data_node = make_terminal_node(ctx);
    vec_pushs(&children, &data_node);


    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintfs(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node colon_node = make_terminal_node(ctx);
    vec_pushs(&children, &colon_node);


    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    
    node->children = children;
    node->kind = CST_DATA_DIR;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}

static bool follows_data_stmt(struct parser_context *ctx) {
    return is_matching_lexeme(ctx, 0, "byte") ||
        is_matching_lexeme(ctx, 0, "bytes") ||
        is_matching_kind(ctx, 0, TOKEN_IDENT);
}

enum parser_result parse_data_stmts(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node data_stmt_node;
    while (follows_data_stmt(ctx)) {
        res = parse_data_stmt(ctx, &data_stmt_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &data_stmt_node);
    }

    

    node->children = children;
    node->kind = CST_DATA_STMTS;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_data_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node child_node;
    if (is_matching_lexeme(ctx, 0, "byte")) {
        res = parse_byte_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        
    } else if (is_matching_lexeme(ctx, 0, "bytes")) {
        res = parse_bytes_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        
    } else {
        res = parse_label_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        
    }
    vec_pushs(&children, &child_node);

    node->children = children;
    node->kind = CST_DATA_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


bool follows_initializer(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_NUM) ||
        is_matching_kind(ctx, 0, TOKEN_ASCII) ||
        is_matching_lexeme(ctx, 0, "{");
}

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    if (!is_matching_lexeme(ctx, 0, "byte")) {
        asprintfs(&ctx->error, "Expected 'byte', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node byte_node = make_terminal_node(ctx);
    vec_pushs(&children, &byte_node);


    if (follows_initializer(ctx)) {
        struct cst_node initializer_node;
        res = parse_initializer(ctx, &initializer_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &initializer_node);
    }

    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    

    node->children = children;
    node->kind = CST_BYTE_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY


    if (!is_matching_lexeme(ctx, 0, "bytes")) {
        asprintfs(&ctx->error, "Expected 'bytes', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node byte_node = make_terminal_node(ctx);
    vec_pushs(&children, &byte_node);


    if (!is_matching_lexeme(ctx, 0, "(")) {
        asprintfs(&ctx->error, "Expected '(', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node lpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &lpar_node);


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        asprintfs(&ctx->error, "Expected number literal, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node num_node = make_terminal_node(ctx);
    vec_pushs(&children, &num_node);


    if (!is_matching_lexeme(ctx, 0, ")")) {
        asprintfs(&ctx->error, "Expected ')', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node rpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &rpar_node);


    if (follows_initializer(ctx)) {
        struct cst_node initializer_node;
        res = parse_initializer(ctx, &initializer_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &initializer_node);
    }

    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    

    node->children = children;
    node->kind = CST_BYTES_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_label_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    
    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintfs(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node ident_node = make_terminal_node(ctx);
    vec_pushs(&children, &ident_node);


    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintfs(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node colon_node = make_terminal_node(ctx);
    vec_pushs(&children, &colon_node);


    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }

    node->children = children;
    node->kind = CST_LABEL_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_initializer(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node child_node;
    if (is_matching_kind(ctx, 0, TOKEN_NUM)) {
        child_node = make_terminal_node(ctx);
    } else if (is_matching_kind(ctx, 0, TOKEN_ASCII)) {
        child_node = make_terminal_node(ctx);
    } else {
        res = parse_byte_initializer(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    }

    vec_pushs(&children, &child_node);

    node->children = children;
    node->kind = CST_INITIALIZER;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_byte_initializer(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY


    if (!is_matching_lexeme(ctx, 0, "{")) {
        asprintfs(&ctx->error, "Expected '{', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node lpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &lpar_node);


    struct cst_node numbers_node;
    res = parse_numbers(ctx, &numbers_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &numbers_node);


    if (!is_matching_lexeme(ctx, 0, "}")) {
        asprintfs(&ctx->error, "Expected '}', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node rpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &rpar_node);

    

    node->children = children;
    node->kind = CST_BYTE_INITIALIZER;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_numbers(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        node->children = children;
        node->kind = CST_NUMBERS;
        return PARSER_OK;
    }

    struct cst_node number_node;
    number_node = make_terminal_node(ctx);
    vec_pushs(&children, &number_node);

    
    while (is_matching_lexeme(ctx, 0, ",")) {
        number_node = make_terminal_node(ctx);
        vec_pushs(&children, &number_node); 

        if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
            asprintfs(&ctx->error, "Expected number literal, found '%s' instead.", current_token_lexeme(ctx));
            goto _error;
        }

        number_node = make_terminal_node(ctx);
        vec_pushs(&children, &number_node); 
    }

    node->children = children;
    node->kind = CST_NUMBERS;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_exec_section(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node exec_dir_node;
    res = parse_exec_dir(ctx, &exec_dir_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &exec_dir_node);


    struct cst_node exec_stmts_node;
    res = parse_exec_stmts(ctx, &exec_stmts_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &exec_stmts_node);
    

    node->children = children;
    node->kind = CST_EXEC_SECTION;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_exec_dir(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, ".EXEC")) {
        asprintfs(&ctx->error, "Expected '.EXEC', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node data_node = make_terminal_node(ctx);
    vec_pushs(&children, &data_node);


    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintfs(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node colon_node = make_terminal_node(ctx);
    vec_pushs(&children, &colon_node);


    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }

    node->children = children;
    node->kind = CST_EXEC_DIR;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


bool follows_exec_stmt(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_INSTR) ||
        is_matching_kind(ctx, 0, TOKEN_MACRO) ||
        is_matching_kind(ctx, 0, TOKEN_IDENT) ||
        is_matching_lexeme(ctx, 0, ".l");
}

enum parser_result parse_exec_stmts(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    
    struct cst_node exec_stmt_node;
    while (follows_exec_stmt(ctx)) {
        res = parse_exec_stmt(ctx, &exec_stmt_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &exec_stmt_node);
    }

    node->children = children;
    node->kind = CST_EXEC_STMTS;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_exec_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    
    struct cst_node child_node;
    if (is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        res = parse_instruction_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    } else if (is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        res = parse_macro_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    } else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        res = parse_label_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    } else {
        res = parse_loc_label_stmt(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    }
    vec_pushs(&children, &child_node);


    node->children = children;
    node->kind = CST_EXEC_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    if (!is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        asprintfs(&ctx->error, "Expected instruction, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node instr_node = make_terminal_node(ctx);
    vec_pushs(&children, &instr_node);

    if (is_matching_lexeme(ctx, 0, "(")) {
        struct cst_node cond_code_node;
        res = parse_condition_code(ctx, &cond_code_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &cond_code_node);
    }


    struct cst_node args_node;
    res = parse_args(ctx, &args_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &args_node);


    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }

    node->children = children;
    node->kind = CST_INSTRUCTION_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_condition_code(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY


    if (!is_matching_lexeme(ctx, 0, "(")) {
        asprintfs(&ctx->error, "Expected '(', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node lpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &lpar_node);



    if (!is_matching_kind(ctx, 0, TOKEN_COND_CODE)) {
        asprintfs(&ctx->error, "Expected condition code, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node cond_code_node = make_terminal_node(ctx);
    vec_pushs(&children, &cond_code_node);


    if (!is_matching_lexeme(ctx, 0, ")")) {
        asprintfs(&ctx->error, "Expected ')', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node rpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &rpar_node);
    

    node->children = children;
    node->kind = CST_CONDITION_CODE;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}



bool follows_arg(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_REG) ||
        is_matching_kind(ctx, 0, TOKEN_SYS_REG) ||
        is_matching_kind(ctx, 0, TOKEN_ADDR_REG) ||
        is_matching_kind(ctx, 0, TOKEN_PORT) ||
        is_matching_kind(ctx, 0, TOKEN_IDENT) ||
        is_matching_lexeme(ctx, 0, "#") ||
        is_matching_lexeme(ctx, 0, ".f") ||
        is_matching_lexeme(ctx, 0, ".b");
}

enum parser_result parse_args(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY



    struct cst_node arg_node;
    if (!follows_arg(ctx)) {
        node->children = children;
        node->kind = CST_ARGS;
        return PARSER_OK;
    }
    res = parse_arg(ctx, &arg_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &arg_node);

    while (is_matching_lexeme(ctx, 0, ",")) {
        arg_node = make_terminal_node(ctx);
        vec_pushs(&children, &arg_node);

       
        res = parse_arg(ctx, &arg_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &arg_node);
    }
    

    node->children = children;
    node->kind = CST_ARGS;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_arg(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node child_node;
    if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        res = parse_label(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        
    } else if (is_matching_lexeme(ctx, 0, "#")) {
        res = parse_immediate(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    } else if (is_matching_lexeme(ctx, 0, ".f") || is_matching_lexeme(ctx, 0, ".b")) {
        res = parse_loc_label(ctx, &child_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
    } else if (follows_arg(ctx)) {
        child_node = make_terminal_node(ctx);
    } else {
        asprintfs(&ctx->error, "Expected argument, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    vec_pushs(&children, &child_node);


    node->children = children;
    node->kind = CST_ARG;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_immediate(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    if (!is_matching_lexeme(ctx, 0, "#")) {
        asprintfs(&ctx->error, "Expected '#', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node hash_node = make_terminal_node(ctx);
    vec_pushs(&children, &hash_node);


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        asprintfs(&ctx->error, "Expected number literal, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node num_node = make_terminal_node(ctx);
    vec_pushs(&children, &num_node);


    node->children = children;
    node->kind = CST_IMMEDIATE;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_label(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
   

    // BODY

    
    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintfs(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node ident_node = make_terminal_node(ctx);
    vec_pushs(&children, &ident_node);

    node->children = children;
    node->kind = CST_LABEL;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}




enum parser_result parse_loc_label(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    struct cst_node direction_node;
    res = parse_direction_dir(ctx, &direction_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &direction_node);

    if (is_matching_lexeme(ctx, 0, "(")) {
        struct cst_node dist_node;
        res = parse_loc_label_dist(ctx, &dist_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &dist_node);
    }


    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintfs(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node ident_node = make_terminal_node(ctx);
    vec_pushs(&children, &ident_node);


    node->children = children;
    node->kind = CST_LOC_LABEL;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_direction_dir(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, ".f") && !is_matching_lexeme(ctx, 0, ".b")) {
        asprintfs(&ctx->error, "Expected '.f' or '.b', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node dir_node = make_terminal_node(ctx);
    vec_pushs(&children, &dir_node);


    node->children = children;
    node->kind = CST_DIRECTION_DIR;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, "(")) {
        asprintfs(&ctx->error, "Expected '(', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node lpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &lpar_node);



    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        asprintfs(&ctx->error, "Expected condition code, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node num_node = make_terminal_node(ctx);
    vec_pushs(&children, &num_node);


    if (!is_matching_lexeme(ctx, 0, ")")) {
        asprintfs(&ctx->error, "Expected ')', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node rpar_node = make_terminal_node(ctx);
    vec_pushs(&children, &rpar_node);


    node->children = children;
    node->kind = CST_LOC_LABEL_DIST;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_macro_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    enum parser_result res;

    // BODY

    
    if (!is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        asprintfs(&ctx->error, "Expected macro, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node macro_node = make_terminal_node(ctx);
    vec_pushs(&children, &macro_node);

    if (is_matching_lexeme(ctx, 0, "(")) {
        struct cst_node cond_code_node;
        res = parse_condition_code(ctx, &cond_code_node);
        if (res == PARSER_ERR) {
            goto _error;
        }
        vec_pushs(&children, &cond_code_node);
    }


    struct cst_node args_node;
    res = parse_args(ctx, &args_node);
    if (res == PARSER_ERR) {
        goto _error;
    }
    vec_pushs(&children, &args_node);


    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }


    node->children = children;
    node->kind = CST_MACRO_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children;
    vec_inits(&children, 10, sizeof(struct cst_node));
    

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, ".l")) {
        asprintfs(&ctx->error, "Expected '.l', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node l_node = make_terminal_node(ctx);
    vec_pushs(&children, &l_node);



    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        asprintfs(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node ident_node = make_terminal_node(ctx);
    vec_pushs(&children, &ident_node);


    if (!is_matching_lexeme(ctx, 0, ":")) {
        asprintfs(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }
    struct cst_node colon_node = make_terminal_node(ctx);
    vec_pushs(&children, &colon_node);


    if (!is_line_terminated(ctx)) {
        asprintfs(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx));
        goto _error;
    }


    node->children = children;
    node->kind = CST_LOC_LABEL_STMT;
    return PARSER_OK;

    _error:
    vec_deinit(&children, &cst_node_deinit);

    return PARSER_ERR;
}
