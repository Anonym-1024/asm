
#include "lexer.h"

#include <ctype.h>
#include "lexer_resources.h"
#include "libs/utilities/utilities.h"
#include "libs/error_handling.h"



struct lexer_context {
    const char *in; //! Reference
    size_t n;
    size_t index;
    struct vector out; //! Owned
    
    size_t line;
    size_t col;
    size_t start_col;

    struct vector buffer; //! Owned

    enum lexer_error_kind error;
};



static bool is_word_start_char(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
}

static bool is_word_char(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_') || (c >= '0' && c <= '9');
}

static bool is_digit_char(char c) {
    return (c >= '0') && (c <= '9');
}

static bool is_hex_digit_char(char c) {
    return ((c >= '0') && (c <= '9')) || ((c >= 'A' && c <= 'F')) || ((c >= 'a') && (c <= 'f'));
}

static bool is_radix_char(char c) {
    return (c == 'd') || (c == 'b') | (c == 'x');
}

static bool is_printable_char(char c) {
    return (c >= ' ') && (c <= 126);
}

static bool is_whitespace_char(char c) {
    return isspace(c);
}

static bool is_punctuation_char(char c) {
    return (c == ',') || (c == '{') || (c == '}') || (c == '[') || (c == ']') || (c == ':') || (c == '(') || (c == ')') || (c == '#');
}




static bool get_char(struct lexer_context *ctx, char *c) {

    if (ctx->index < ctx->n) {
        *c = ctx->in[ctx->index];
        return true;   
    }

    return false;
}

static void pop_char(struct lexer_context *ctx) {
    ctx->index += 1;
    ctx->col += 1;
}


enum lexer_result add_token(struct lexer_context *ctx, enum token_kind kind) {
    

    struct token t;
    t.kind = kind;
    t.line = ctx->line;
    t.col = ctx->start_col;

    t.lexeme = ctx->buffer.ptr;
    

    
    try_else(vec_push(&ctx->out, &t), VEC_OK, goto _error);
    
    try_else(vec_init(&ctx->buffer, 10, sizeof(char)), VEC_OK, goto _error);

    return LEX_OK;


_error:
    
    ctx->error = LEX_MEM_ERR;
    return LEX_ERR;

}



static enum lexer_result read_comment(struct lexer_context *ctx) {
    char c;
    while (get_char(ctx, &c) == true && c != '\n') {
        pop_char(ctx);
    }
    return LEX_OK;
}


static enum lexer_result read_new_line(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;

    char c = '\n';
    try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    try_else(add_token(ctx, TOKEN_PUNCT), LEX_OK, goto _error);
    
    pop_char(ctx);

    ctx->col = 1;
    ctx->line += 1;
    
    return LEX_OK;

_error:

    ctx->error = LEX_MEM_ERR;
    return LEX_ERR;
}


static enum lexer_result read_whitespace(struct lexer_context *ctx) {
    pop_char(ctx);
    return LEX_OK;
}


static enum lexer_result read_punctuation(struct lexer_context *ctx, char c) {
    ctx->start_col = ctx->col;

    try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
    pop_char(ctx);

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    try_else(add_token(ctx, TOKEN_PUNCT), LEX_OK, goto _error);
    return LEX_OK;


_error:

    return LEX_ERR;
}


static bool is_directive(char *s) {
    for (int i = 0; i < directives_count; i++) {
        if (strcmp(s, directives[i]) == 0) {
            return true;
        }
    }
    return false;
}

static enum lexer_result read_directive(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;

    char c;
    get_char(ctx, &c);
    try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
    pop_char(ctx);

    
    while (get_char(ctx, &c) == true && is_word_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
        pop_char(ctx);
    }

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    if (!is_directive(ctx->buffer.ptr)) {
        ctx->error = LEX_INV_DIR;
        return LEX_ERR;
    }

    try_else(add_token(ctx, TOKEN_DIR), LEX_OK, goto _error);

    return LEX_OK;

_error:
    return LEX_ERR;
}


static bool is_macro(char *s) {
    for (int i = 0; i < macros_count; i++) {
        if (strcmp(s, macros[i]) == 0) {
            return true;
        }
    }
    return false;
}

static enum lexer_result read_macro(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;

    char c;
    get_char(ctx, &c);
    try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
    pop_char(ctx);

    
    while (get_char(ctx, &c) == true && is_word_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
        pop_char(ctx);
    }
    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);


    if (!is_macro(ctx->buffer.ptr)) {
        ctx->error = LEX_INV_MACRO;
        return LEX_ERR;
    }

    try_else(add_token(ctx, TOKEN_MACRO), LEX_OK, goto _error);

    return LEX_OK;

_error:
    return LEX_ERR;
}

static bool is_instruction(const char *s) {
    for (int i = 0; i < instructions_count; i++) {
        if (strcmp(s, instructions[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_data_unit(const char *s) {
    for (int i = 0; i < data_units_count; i++) {
        if (strcmp(s, data_units[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_register(const char *s) {
    for (int i = 0; i < registers_count; i++) {
        if (strcmp(s, registers[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_address_register(const char *s) {
    for (int i = 0; i < address_registers_count; i++) {
        if (strcmp(s, address_registers[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_condition_code(const char *s) {
    for (int i = 0; i < condition_codes_count; i++) {
        if (strcmp(s, condition_codes[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_system_register(const char *s){
    for (int i = 0; i < system_registers_count; i++) {
        if (strcmp(s, system_registers[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_port(const char *s){
    for (int i = 0; i < ports_count; i++) {
        if (strcmp(s, ports[i]) == 0) {
            return true;
        }
    }
    return false;
}


static enum token_kind get_word_kind(const char *s) {
    if (is_instruction(s)) {
        return TOKEN_INSTR;
    } else if (is_data_unit(s)) {
        return TOKEN_DATA_UNIT;
    } else if (is_register(s)) {
        return TOKEN_REG;
    } else if (is_address_register(s)) {
        return TOKEN_ADDR_REG;
    } else if (is_port(s)) {
        return TOKEN_PORT;
    } else if (is_system_register(s)) {
        return TOKEN_SYS_REG;
    } else if (is_condition_code(s)) {
        return TOKEN_COND_CODE;
    } else {
        return TOKEN_IDENT;
    }
}

static enum lexer_result read_word(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    char c;
    while (get_char(ctx, &c) == true && is_word_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
        pop_char(ctx);
    }
    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    enum token_kind kind = get_word_kind(ctx->buffer.ptr);
    

    try_else(add_token(ctx, kind), LEX_OK, goto _error);

    return LEX_OK;

_error:
    return LEX_ERR;
}


static enum lexer_result read_ascii(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    pop_char(ctx);

    char c;
    while(get_char(ctx, &c) == true && c != '"') {
        if (!is_printable_char(c)) {
            ctx->error = LEX_INV_ASCII_LIT;
            return LEX_ERR;
        }
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
        pop_char(ctx);
    }
    if (c != '"') {
        ctx->error = LEX_UNTERM_ASCII_LIT;
        return LEX_ERR;
    }
    pop_char(ctx);

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    try_else(add_token(ctx, TOKEN_ASCII), LEX_OK, goto _error);

    return LEX_OK;

_error:
    return LEX_ERR;

}



static enum lexer_result read_number(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    char c;
    get_char(ctx, &c);
    try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
    pop_char(ctx);

    while (get_char(ctx, &c) == true && is_hex_digit_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
        pop_char(ctx);
    }

    if (get_char(ctx, &c) == true && is_radix_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, goto _error);
        pop_char(ctx);
    }

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    try_else(add_token(ctx, TOKEN_NUM), LEX_OK, goto _error);



    return LEX_OK;

_error:
    return LEX_ERR;
}



void token_deinit(struct token *ptr) {
    free(ptr->lexeme);
}

void _token_deinit(void *ptr) {
    token_deinit(ptr);
}


enum lexer_result tokenise(const char *in, size_t n, struct vector *out, struct lexer_error *error) {

    
    struct lexer_context ctx = {
        .in = in,
        .n = n,
        .index = 0,

        .line = 1,
        .col = 1,
        .start_col = 1,

        .error = LEX_UNKNOWN_ERR,

        .buffer.ptr = NULL,
        .out.ptr = NULL
    }; 
    char *eof_lexeme = NULL;
    

    try_else(vec_init(&ctx.buffer, 10, sizeof(char)), VEC_OK, goto _error);
    try_else(vec_init(&ctx.out, 100, sizeof(struct token)), VEC_OK, goto _error);


    char c;

    while (get_char(&ctx, &c) == true) {
        if (c == ';') {
            try_else(read_comment(&ctx), LEX_OK, goto _error);
            
        } else if (c == '\n') {
            try_else(read_new_line(&ctx), LEX_OK, goto _error);
            
        } else if (is_whitespace_char(c)) {
            try_else(read_whitespace(&ctx), LEX_OK, goto _error);
            
        } else if (is_punctuation_char(c)) {
            try_else(read_punctuation(&ctx, c), LEX_OK, goto _error);
            
        } else if (c == '.') {
            try_else(read_directive(&ctx), LEX_OK, goto _error);
            
        } else if (c == '!') {
            try_else(read_macro(&ctx), LEX_OK, goto _error);
            
        } else if (c == '"') {
            try_else(read_ascii(&ctx), LEX_OK, goto _error);
            
        } else if (is_digit_char(c) || c == '-') {
            try_else(read_number(&ctx), LEX_OK, goto _error);
            
        } else if (is_word_start_char(c)) {
            try_else(read_word(&ctx), LEX_OK, goto _error);
            
        } else {
            ctx.error = LEX_INV_CHAR;
            goto _error;
        }
    }

    
    struct token eof = {
        .kind = TOKEN_EOF,
        .line = 0,
        .col = 0,
        .lexeme = eof_lexeme
    };
    if ((eof.lexeme = malloc(sizeof(char) * 1)) == NULL) {
        goto _error;
    }
    strcpy(eof.lexeme, "");

    try_else(vec_push(&ctx.out, &eof), VEC_OK, goto _error);


    *out = ctx.out;

    vec_deinit(&ctx.buffer, NULL);

    return LEX_OK;

_error:
    
    vec_deinit(&ctx.buffer, NULL);
    vec_deinit(&ctx.out, &_token_deinit);
    error->line = ctx.line;
    error->col = ctx.start_col;
    error->kind = ctx.error;
    return LEX_ERR;
}



int get_token_desc(struct token *t, char **out) {
    const char *kind = NULL;

    switch (t->kind) {
        case TOKEN_PUNCT:
        kind = "punctuation";
        break;

        case TOKEN_ADDR_REG:
        kind = "address register";
        break;

        case TOKEN_ASCII:
        kind = "ascii literal";
        break;

        case TOKEN_COND_CODE:
        kind = "condition code";
        break;

        case TOKEN_DATA_UNIT:
        kind = "data unit";
        break;

        case TOKEN_DIR:
        kind = "directive";
        break;

        case TOKEN_IDENT:
        kind = "identifier";
        break;

        case TOKEN_INSTR:
        kind = "instruction";
        break;

        case TOKEN_MACRO:
        kind = "macro";
        break;

        case TOKEN_NUM:
        kind = "number literal";
        break;

        case TOKEN_EOF:
        kind = "end of file";
        break;

        case TOKEN_PORT:
        kind = "port";
        break;

        case TOKEN_REG:
        kind = "register";
        break;

        case TOKEN_SYS_REG:
        kind = "system register";
        break;
    }


    *out = NULL;
    if (asprintf(out, "TOKEN [%ld:%ld]: { Kind: %s; Lexeme: %s }", t->line, t->col, kind, t->lexeme) == -1) {
        goto _error;
    }
    return 0;

_error:
    free(*out);
    return -1;

    
}
