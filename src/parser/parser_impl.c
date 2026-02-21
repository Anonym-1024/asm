


#include "parser_impl.h"
#include "libs/utilities/utilities.h"
#include <time.h>


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

/// Pops a copy of current token
static enum parser_result pop_token(struct parser_context *ctx, struct token *t) {
    *t = ctx->in[ctx->index];
    t->lexeme = malloc(strlen(t->lexeme) * sizeof(char) + 1);
    if (t->lexeme == NULL) {
        return PARSER_ERR;
    }
    strcpy(t->lexeme, ctx->in[ctx->index].lexeme);
    ctx->index += 1;
    return PARSER_OK;
}

static const char *current_token_lexeme(struct parser_context *ctx) {
    if (ctx->index < ctx->n) {
        return ctx->in[ctx->index].lexeme;
    }
    return "End of file.";
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





static enum parser_result make_terminal_node(struct parser_context *ctx, struct cst_node *node) {
    node->kind = CST_TERMINAL;
    try_else(pop_token(ctx, &node->terminal), PARSER_OK, goto _error);
    return PARSER_OK;

    _error:
    return PARSER_ERR;
}




static struct cst_node null_cst_node() {
    struct cst_node n;
    n.children.ptr = NULL;
    n.terminal.lexeme = NULL;
    n.kind = CST_TERMINAL;
    return n;
}










enum parser_result parse_file(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();

    struct cst_node sections_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    

    // BODY

    while (is_matching_lexeme(ctx, 0, "\n")) {
        ctx->index += 1;
    }

    
    try_else(parse_sections(ctx, &sections_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &sections_node), VEC_OK, goto _error);
    sections_node = null_cst_node();



    while (is_matching_lexeme(ctx, 0, "\n")) {
        ctx->index += 1;
    }

    if (!is_matching_kind(ctx, 0, TOKEN_EOF)) {
        if (asprintf(&ctx->error, "Expected end of file ") == -1) {
            free(ctx->error);
        }
        goto _error;
    }

    


    node->children = children;
    node->kind = CST_FILE;
    return PARSER_OK;


_error:
    cst_node_deinit(&sections_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


bool follows_section(struct parser_context *ctx) {
    return is_matching_lexeme(ctx, 0, ".DATA") || is_matching_lexeme(ctx, 0, ".EXEC");
}

enum parser_result parse_sections(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node section_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    while (follows_section(ctx)) {
        try_else(parse_section(ctx, &section_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &section_node), VEC_OK, goto _error);
        section_node = null_cst_node();
    }
    
    
    node->children = children;
    node->kind = CST_SECTIONS;
    return PARSER_OK;


_error:
    cst_node_deinit(&section_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_section(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node child_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY


    
    if (is_matching_lexeme(ctx, 0, ".DATA")) {
        try_else(parse_data_section(ctx, &child_node), PARSER_OK, goto _error);
        
        

    } else {
        try_else(parse_exec_section(ctx, &child_node), PARSER_OK, goto _error);
        
        
    }
    try_else(vec_push(&children, &child_node), VEC_OK, goto _error);
    child_node = null_cst_node();
    
    node->children = children;
    node->kind = CST_SECTION;
    return PARSER_OK;



_error:
    cst_node_deinit(&child_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_data_section(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node data_dir_node = null_cst_node();
    struct cst_node data_stmts_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    try_else(parse_data_dir(ctx, &data_dir_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &data_dir_node), VEC_OK, goto _error);
    data_dir_node = null_cst_node();


    
    try_else(parse_data_stmts(ctx, &data_stmts_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &data_stmts_node), VEC_OK, goto _error);
    data_stmts_node = null_cst_node();




    node->children = children;
    node->kind = CST_DATA_SECTION;
    return PARSER_OK;

_error:
    cst_node_deinit(&data_dir_node);
    cst_node_deinit(&data_stmts_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_data_dir(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    
    struct cst_node data_node = null_cst_node();
    struct cst_node colon_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    // BODY

    if (!is_matching_lexeme(ctx, 0, ".DATA")) {
        if (asprintf(&ctx->error, "Expected '.DATA', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &data_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &data_node), VEC_OK, goto _error);
    data_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ":")) {
        if (asprintf(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &colon_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &colon_node), VEC_OK, goto _error);
    colon_node = null_cst_node();


    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    
    node->children = children;
    node->kind = CST_DATA_DIR;
    return PARSER_OK;

_error:
    cst_node_deinit(&data_node);
    cst_node_deinit(&colon_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}

static bool follows_data_stmt(struct parser_context *ctx) {
    return is_matching_lexeme(ctx, 0, "byte") ||
        is_matching_lexeme(ctx, 0, "bytes") ||
        is_matching_kind(ctx, 0, TOKEN_IDENT);
}

enum parser_result parse_data_stmts(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node data_stmt_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

   
    while (follows_data_stmt(ctx)) {
        try_else(parse_data_stmt(ctx, &data_stmt_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &data_stmt_node), VEC_OK, goto _error);
        data_stmt_node = null_cst_node();
    }

    

    node->children = children;
    node->kind = CST_DATA_STMTS;
    return PARSER_OK;

_error:
    cst_node_deinit(&data_stmt_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_data_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node child_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (is_matching_lexeme(ctx, 0, "byte")) {
        try_else(parse_byte_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
        
    } else if (is_matching_lexeme(ctx, 0, "bytes")) {
        try_else(parse_bytes_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
        
    } else {
        try_else(parse_label_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
        
    }
    try_else(vec_push(&children, &child_node), VEC_OK, goto _error);
    child_node = null_cst_node();

    node->children = children;
    node->kind = CST_DATA_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&child_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


bool follows_initializer(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_NUM) ||
        is_matching_kind(ctx, 0, TOKEN_ASCII) ||
        is_matching_lexeme(ctx, 0, "{");
}

enum parser_result parse_byte_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node byte_node = null_cst_node();
    struct cst_node initializer_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    // BODY

    if (!is_matching_lexeme(ctx, 0, "byte")) {
        if (asprintf(&ctx->error, "Expected 'byte', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &byte_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &byte_node), VEC_OK, goto _error);
    byte_node = null_cst_node();


    if (follows_initializer(ctx)) {
        
        try_else(parse_initializer(ctx, &initializer_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &initializer_node), VEC_OK, goto _error);
        initializer_node = null_cst_node();
    }

    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    

    node->children = children;
    node->kind = CST_BYTE_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&byte_node);
    cst_node_deinit(&initializer_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_bytes_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node byte_node = null_cst_node();
    struct cst_node lpar_node = null_cst_node();
    struct cst_node num_node = null_cst_node();
    struct cst_node rpar_node = null_cst_node();
    struct cst_node initializer_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY


    if (!is_matching_lexeme(ctx, 0, "bytes")) {
        if (asprintf(&ctx->error, "Expected 'bytes', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &byte_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &byte_node), VEC_OK, goto _error);
    byte_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, "(")) {
        if (asprintf(&ctx->error, "Expected '(', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &lpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &lpar_node), VEC_OK, goto _error);
    lpar_node = null_cst_node();


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        if (asprintf(&ctx->error, "Expected number literal, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &num_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &num_node), VEC_OK, goto _error);
    num_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ")")) {
        if (asprintf(&ctx->error, "Expected ')', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &rpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &rpar_node), VEC_OK, goto _error);
    rpar_node = null_cst_node();


    if (follows_initializer(ctx)) {
        try_else(parse_initializer(ctx, &initializer_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &initializer_node), VEC_OK, goto _error);
        initializer_node = null_cst_node();
    }

    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    

    node->children = children;
    node->kind = CST_BYTES_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&byte_node);
    cst_node_deinit(&initializer_node);
    cst_node_deinit(&lpar_node);
    cst_node_deinit(&num_node);
    cst_node_deinit(&rpar_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_label_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    
    struct cst_node ident_node = null_cst_node();
    struct cst_node colon_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        if (asprintf(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &ident_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &ident_node), VEC_OK, goto _error);
    ident_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ":")) {
        if (asprintf(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &colon_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &colon_node), VEC_OK, goto _error);
    colon_node = null_cst_node();


    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }

    node->children = children;
    node->kind = CST_LABEL_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&colon_node);
    cst_node_deinit(&ident_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_initializer(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node child_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (is_matching_kind(ctx, 0, TOKEN_NUM)) {
        try_else(make_terminal_node(ctx, &child_node), PARSER_OK, goto _error);
    } else if (is_matching_kind(ctx, 0, TOKEN_ASCII)) {
        try_else(make_terminal_node(ctx, &child_node), PARSER_OK, goto _error);
    } else {
        try_else(parse_byte_initializer(ctx, &child_node), PARSER_OK, goto _error);
        
    }

    try_else(vec_push(&children, &child_node), VEC_OK, goto _error);
    child_node = null_cst_node();

    node->children = children;
    node->kind = CST_INITIALIZER;
    return PARSER_OK;

_error:
    cst_node_deinit(&child_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_byte_initializer(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node lpar_node = null_cst_node();
    struct cst_node numbers_node = null_cst_node();
    struct cst_node rpar_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY


    if (!is_matching_lexeme(ctx, 0, "{")) {
        if (asprintf(&ctx->error, "Expected '{', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &lpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &lpar_node), VEC_OK, goto _error);
    lpar_node = null_cst_node();


    
    try_else(parse_numbers(ctx, &numbers_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &numbers_node), VEC_OK, goto _error);
    numbers_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, "}")) {
        if (asprintf(&ctx->error, "Expected '}', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &rpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &rpar_node), VEC_OK, goto _error);
    rpar_node = null_cst_node();

    

    node->children = children;
    node->kind = CST_BYTE_INITIALIZER;
    return PARSER_OK;

_error:
    cst_node_deinit(&lpar_node);
    cst_node_deinit(&numbers_node);
    cst_node_deinit(&numbers_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_numbers(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node number_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    // BODY

    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        node->children = children;
        node->kind = CST_NUMBERS;
        return PARSER_OK;
    }

    
    try_else(make_terminal_node(ctx, &number_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &number_node), VEC_OK, goto _error);
    number_node = null_cst_node();

    
    while (is_matching_lexeme(ctx, 0, ",")) {
        try_else(make_terminal_node(ctx, &number_node), PARSER_OK, goto _error);
        try_else(vec_push(&children, &number_node), VEC_OK, goto _error); 
        number_node = null_cst_node();

        if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
            if (asprintf(&ctx->error, "Expected number literal, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
            goto _error;
        }

        try_else(make_terminal_node(ctx, &number_node), PARSER_OK, goto _error);
        try_else(vec_push(&children, &number_node), VEC_OK, goto _error);
        number_node = null_cst_node(); 
    }

    node->children = children;
    node->kind = CST_NUMBERS;
    return PARSER_OK;

_error:
    cst_node_deinit(&number_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_exec_section(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node exec_dir_node = null_cst_node();
    struct cst_node exec_stmts_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    try_else(parse_exec_dir(ctx, &exec_dir_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &exec_dir_node), VEC_OK, goto _error);
    exec_dir_node = null_cst_node();


    
    try_else(parse_exec_stmts(ctx, &exec_stmts_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &exec_stmts_node), VEC_OK, goto _error);
    exec_stmts_node = null_cst_node();
    

    node->children = children;
    node->kind = CST_EXEC_SECTION;
    return PARSER_OK;

_error:
    cst_node_deinit(&exec_dir_node);
    cst_node_deinit(&exec_stmts_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_exec_dir(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node data_node = null_cst_node();
    struct cst_node colon_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    // BODY

    
    if (!is_matching_lexeme(ctx, 0, ".EXEC")) {
        if (asprintf(&ctx->error, "Expected '.EXEC', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &data_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &data_node), VEC_OK, goto _error);
    data_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ":")) {
        if (asprintf(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &colon_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &colon_node), VEC_OK, goto _error);
    colon_node = null_cst_node();


    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }

    node->children = children;
    node->kind = CST_EXEC_DIR;
    return PARSER_OK;

_error:
    cst_node_deinit(&data_node);
    cst_node_deinit(&colon_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


bool follows_exec_stmt(struct parser_context *ctx) {
    return is_matching_kind(ctx, 0, TOKEN_INSTR) ||
        is_matching_kind(ctx, 0, TOKEN_MACRO) ||
        is_matching_kind(ctx, 0, TOKEN_IDENT) ||
        is_matching_lexeme(ctx, 0, ".l");
}

enum parser_result parse_exec_stmts(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node exec_stmt_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    
    while (follows_exec_stmt(ctx)) {
        try_else(parse_exec_stmt(ctx, &exec_stmt_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &exec_stmt_node), VEC_OK, goto _error);
        exec_stmt_node = null_cst_node();
    }

    node->children = children;
    node->kind = CST_EXEC_STMTS;
    return PARSER_OK;

_error:
    cst_node_deinit(&exec_stmt_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_exec_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node child_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    
    if (is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        try_else(parse_instruction_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
        
    } else if (is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        try_else(parse_macro_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
        
    } else if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        try_else(parse_label_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
    } else {
        try_else(parse_loc_label_stmt(ctx, &child_node), PARSER_OK, goto _error);
        
    }
    try_else(vec_push(&children, &child_node), VEC_OK, goto _error);
    child_node = null_cst_node();


    node->children = children;
    node->kind = CST_EXEC_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&child_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_instruction_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node instr_node = null_cst_node();
    struct cst_node cond_code_node = null_cst_node();
    struct cst_node args_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    // BODY

    if (!is_matching_kind(ctx, 0, TOKEN_INSTR)) {
        if (asprintf(&ctx->error, "Expected instruction, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &instr_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &instr_node), VEC_OK, goto _error);
    instr_node = null_cst_node();

    if (is_matching_lexeme(ctx, 0, "(")) {
       
        try_else(parse_condition_code(ctx, &cond_code_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &cond_code_node), VEC_OK, goto _error);
        cond_code_node = null_cst_node();
    }


    
    try_else(parse_args(ctx, &args_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &args_node), VEC_OK, goto _error);
    args_node = null_cst_node();


    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }

    node->children = children;
    node->kind = CST_INSTRUCTION_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&instr_node);
    cst_node_deinit(&args_node);
    cst_node_deinit(&cond_code_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_condition_code(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node lpar_node = null_cst_node();
    struct cst_node cond_code_node = null_cst_node();
    struct cst_node rpar_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);
    // BODY


    if (!is_matching_lexeme(ctx, 0, "(")) {
        if (asprintf(&ctx->error, "Expected '(', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &lpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &lpar_node), VEC_OK, goto _error);
    lpar_node = null_cst_node();



    if (!is_matching_kind(ctx, 0, TOKEN_COND_CODE)) {
        if (asprintf(&ctx->error, "Expected condition code, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &cond_code_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &cond_code_node), VEC_OK, goto _error);
    cond_code_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ")")) {
        if (asprintf(&ctx->error, "Expected ')', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &rpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &rpar_node), VEC_OK, goto _error);
    rpar_node = null_cst_node();
    

    node->children = children;
    node->kind = CST_CONDITION_CODE;
    return PARSER_OK;

_error:
    cst_node_deinit(&lpar_node);
    cst_node_deinit(&cond_code_node);
    cst_node_deinit(&rpar_node);
    vec_deinit(&children, &_cst_node_deinit);

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
    struct vector children = null_vector();
    
    

    struct cst_node arg_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY



    
    if (!follows_arg(ctx)) {
        node->children = children;
        node->kind = CST_ARGS;
        return PARSER_OK;
    }
    try_else(parse_arg(ctx, &arg_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &arg_node), VEC_OK, goto _error);
    arg_node = null_cst_node();

    while (is_matching_lexeme(ctx, 0, ",")) {
        try_else(make_terminal_node(ctx, &arg_node), PARSER_OK, goto _error);
        try_else(vec_push(&children, &arg_node), VEC_OK, goto _error);
        arg_node = null_cst_node();

       
        try_else(parse_arg(ctx, &arg_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &arg_node), VEC_OK, goto _error);
        arg_node = null_cst_node();
    }
    

    node->children = children;
    node->kind = CST_ARGS;
    return PARSER_OK;

_error:
    cst_node_deinit(&arg_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_arg(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node child_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        try_else(parse_label(ctx, &child_node), PARSER_OK, goto _error);
        
        
    } else if (is_matching_lexeme(ctx, 0, "#")) {
        try_else(parse_immediate(ctx, &child_node), PARSER_OK, goto _error);
        
    } else if (is_matching_lexeme(ctx, 0, ".f") || is_matching_lexeme(ctx, 0, ".b")) {
        try_else(parse_loc_label(ctx, &child_node), PARSER_OK, goto _error);
        
    } else if (follows_arg(ctx)) {
       try_else(make_terminal_node(ctx, &child_node), PARSER_OK, goto _error);
    } else {
        if (asprintf(&ctx->error, "Expected argument, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(vec_push(&children, &child_node), VEC_OK, goto _error);
    child_node = null_cst_node();


    node->children = children;
    node->kind = CST_ARG;
    return PARSER_OK;

_error:
    cst_node_deinit(&child_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_immediate(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    
    struct cst_node hash_node = null_cst_node();
    struct cst_node num_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    if (!is_matching_lexeme(ctx, 0, "#")) {
        if (asprintf(&ctx->error, "Expected '#', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &hash_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &hash_node), VEC_OK, goto _error);
    hash_node = null_cst_node();


    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        if (asprintf(&ctx->error, "Expected number literal, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &num_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &num_node), VEC_OK, goto _error);
    num_node = null_cst_node();


    node->children = children;
    node->kind = CST_IMMEDIATE;
    return PARSER_OK;

_error:
    cst_node_deinit(&hash_node);
    cst_node_deinit(&num_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}


enum parser_result parse_label(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
   
    struct cst_node ident_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        if (asprintf(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &ident_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &ident_node), VEC_OK, goto _error);
    ident_node = null_cst_node();

    node->children = children;
    node->kind = CST_LABEL;
    return PARSER_OK;

_error:
    cst_node_deinit(&ident_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}




enum parser_result parse_loc_label(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node direction_node = null_cst_node();
    struct cst_node dist_node = null_cst_node();
    struct cst_node ident_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    try_else(parse_direction_dir(ctx, &direction_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &direction_node), VEC_OK, goto _error);
    direction_node = null_cst_node();


    if (is_matching_lexeme(ctx, 0, "(")) {
        
        try_else(parse_loc_label_dist(ctx, &dist_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &dist_node), VEC_OK, goto _error);
        dist_node = null_cst_node();
    }


    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        if (asprintf(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &ident_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &ident_node), VEC_OK, goto _error);
    ident_node = null_cst_node();


    node->children = children;
    node->kind = CST_LOC_LABEL;
    return PARSER_OK;

_error:
    cst_node_deinit(&direction_node);
    cst_node_deinit(&dist_node);
    cst_node_deinit(&ident_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_direction_dir(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    
    struct cst_node dir_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, ".f") && !is_matching_lexeme(ctx, 0, ".b")) {
        if (asprintf(&ctx->error, "Expected '.f' or '.b', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &dir_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &dir_node), VEC_OK, goto _error);
    dir_node = null_cst_node();


    node->children = children;
    node->kind = CST_DIRECTION_DIR;
    return PARSER_OK;

_error:
    cst_node_deinit(&dir_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_loc_label_dist(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    
    struct cst_node lpar_node = null_cst_node();
    struct cst_node num_node = null_cst_node();
    struct cst_node rpar_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, "(")) {
        if (asprintf(&ctx->error, "Expected '(', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &lpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &lpar_node), VEC_OK, goto _error);
    lpar_node = null_cst_node();



    if (!is_matching_kind(ctx, 0, TOKEN_NUM)) {
        if (asprintf(&ctx->error, "Expected condition code, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &num_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &num_node), VEC_OK, goto _error);
    num_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ")")) {
        if (asprintf(&ctx->error, "Expected ')', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &rpar_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &rpar_node), VEC_OK, goto _error);
    rpar_node = null_cst_node();


    node->children = children;
    node->kind = CST_LOC_LABEL_DIST;
    return PARSER_OK;

_error:
    cst_node_deinit(&lpar_node);
    cst_node_deinit(&num_node);
    cst_node_deinit(&rpar_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_macro_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    

    struct cst_node macro_node = null_cst_node();
    struct cst_node cond_code_node = null_cst_node();
    struct cst_node args_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (!is_matching_kind(ctx, 0, TOKEN_MACRO)) {
        if (asprintf(&ctx->error, "Expected macro, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &macro_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &macro_node), VEC_OK, goto _error);
    macro_node = null_cst_node();

    if (is_matching_lexeme(ctx, 0, "(")) {
        try_else(parse_condition_code(ctx, &cond_code_node), PARSER_OK, goto _error);
        
        try_else(vec_push(&children, &cond_code_node), VEC_OK, goto _error);
        cond_code_node = null_cst_node();
    }


    
    try_else(parse_args(ctx, &args_node), PARSER_OK, goto _error);
    
    try_else(vec_push(&children, &args_node), VEC_OK, goto _error);
    args_node = null_cst_node();


    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }


    node->children = children;
    node->kind = CST_MACRO_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&args_node);
    cst_node_deinit(&macro_node);
    cst_node_deinit(&cond_code_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}

enum parser_result parse_loc_label_stmt(struct parser_context *ctx, struct cst_node *node) {
    struct vector children = null_vector();
    
    
    struct cst_node l_node = null_cst_node();
    struct cst_node ident_node = null_cst_node();
    struct cst_node colon_node = null_cst_node();

    try_else(vec_init(&children, 10, sizeof(struct cst_node)), VEC_OK, goto _error);

    // BODY

    
    if (!is_matching_lexeme(ctx, 0, ".l")) {
        if (asprintf(&ctx->error, "Expected '.l', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &l_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &l_node), VEC_OK, goto _error);
    l_node = null_cst_node();



    if (!is_matching_kind(ctx, 0, TOKEN_IDENT)) {
        if (asprintf(&ctx->error, "Expected identifier, found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &ident_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &ident_node), VEC_OK, goto _error);
    ident_node = null_cst_node();


    if (!is_matching_lexeme(ctx, 0, ":")) {
        if (asprintf(&ctx->error, "Expected ':', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }
    try_else(make_terminal_node(ctx, &colon_node), PARSER_OK, goto _error);
    try_else(vec_push(&children, &colon_node), VEC_OK, goto _error);
    colon_node = null_cst_node();


    if (!is_line_terminated(ctx)) {
        if (asprintf(&ctx->error, "Expected 'Line break', found '%s' instead.", current_token_lexeme(ctx)) == -1) { free(ctx->error); }
        goto _error;
    }


    node->children = children;
    node->kind = CST_LOC_LABEL_STMT;
    return PARSER_OK;

_error:
    cst_node_deinit(&l_node);
    cst_node_deinit(&colon_node);
    cst_node_deinit(&ident_node);
    vec_deinit(&children, &_cst_node_deinit);

    return PARSER_ERR;
}
