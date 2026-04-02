
#include "lexer.h"

#include <ctype.h>
#include "libs/vector/vector.h"
#include "libs/hashmap/hashmap.h"
#include "libs/error_handling.h"
#include <errno.h>

struct lexer_context {
    FILE *in; //! Reference
    int c;
    struct vector out; //! Owned
    
    uint32_t line;
    uint16_t col;
    uint16_t start_col;

    struct vector buffer; //! Owned
    bool _buffer;

    char error_msg[ERR_MSG_LEN];

    struct hashmap dir_map;
    struct hashmap instr_map;
    //struct hashmap macro_map;
    struct hashmap reg_map;
    struct hashmap sys_reg_map;
    struct hashmap addr_reg_map;
    struct hashmap port_map;
    struct hashmap data_unit_map;
    struct hashmap punct_map;
    struct hashmap cond_code_map;
    
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
    return (c == ',') || (c == '{') || (c == '}') || (c == ':') || (c == '(') || (c == ')') || (c == '#') || (c == '=');
}






static void next(struct lexer_context *ctx) {
    ctx->c = getc(ctx->in);
    ctx->col += 1;
}


static enum lexer_result add_lexical_token(struct lexer_context *ctx, enum token_kind kind) {
    

    struct token t;
    t.kind = kind;
    //t.line = ctx->line;
    t.col = ctx->start_col;

    t.lexeme = ctx->buffer.ptr;
    

    
    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR);
    ctx->_buffer = false;
    try_else(vec_init(&ctx->buffer, 6, sizeof(char)), VEC_OK, return LEX_ERR);
    ctx->_buffer = true;

    return LEX_OK;




}



static enum lexer_result read_comment(struct lexer_context *ctx) {
    
    while (ctx->c != EOF && ctx->c != '\n') {
        next(ctx);
    }
    return LEX_OK;
}


static enum lexer_result read_new_line(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;

    
    struct token t = {
        .kind = TOKEN_PUNCT,
        //.line = ctx->line,
        .col = ctx->start_col,
        .punct = PUNCT_NEWLINE
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR);
    
    next(ctx);
    ctx->col = 1;
    ctx->line += 1;
    
    return LEX_OK;


}


static enum lexer_result read_whitespace(struct lexer_context *ctx) {
    next(ctx);
    return LEX_OK;
}


static enum lexer_result read_punctuation(struct lexer_context *ctx, char c) {
    ctx->start_col = ctx->col;

    try_else(vec_push(&ctx->buffer, &c), VEC_OK, return LEX_ERR);
    next(ctx);

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, return LEX_ERR);

    
    enum punctuation_token p;
    hashmap_get(&ctx->punct_map, ctx->buffer.ptr, &p);

    struct token t = {
        .kind = TOKEN_PUNCT,
        //.line = ctx->line,
        .col = ctx->start_col,
        .punct = p
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR);
    vec_empty(&ctx->buffer);
    return LEX_OK;



}



static enum lexer_result read_directive(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;

    
    
    try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, goto _error);
    next(ctx);

    
    while (ctx->c != EOF && is_word_char(ctx->c)) {
        try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, goto _error);
        next(ctx);
    }

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);


    
    enum directive_token d;
    if (hashmap_get(&ctx->dir_map, ctx->buffer.ptr, &d) != HMAP_OK) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "'%.20s' is not a valid directive.", (char*)ctx->buffer.ptr);
        goto _error;
    }
     

    struct token t = {
        .kind = TOKEN_DIR,
        //.line = ctx->line,
        .col = ctx->start_col,
        .dir = d
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, goto _error);
    vec_empty(&ctx->buffer);

    return LEX_OK;

_error:
   
    return LEX_ERR;
}




static enum lexer_result read_word(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    
    while (ctx->c != EOF && is_word_char(ctx->c)) {
        try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, goto _error);
        next(ctx);
    }
    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, goto _error);

    

    struct token t = {
        //.line = ctx->line,
        .col = ctx->start_col
    };
    
    unsigned int x;
    
    if (hashmap_get(&ctx->instr_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_INSTR;
        
        t.instr = x;
    } else if (hashmap_get(&ctx->reg_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_REG;
        
        t.reg = x;
    } else if (hashmap_get(&ctx->sys_reg_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_SYS_REG;
        
        t.sys_reg = x;
    } else if (hashmap_get(&ctx->addr_reg_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_ADDR_REG;
        
        t.addr_reg = x;
    } else if (hashmap_get(&ctx->port_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_PORT;
        
        t.port = x;
    } else if (hashmap_get(&ctx->data_unit_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_DATA_UNIT;
        
        t.data_unit = x;
    } else if (hashmap_get(&ctx->cond_code_map, ctx->buffer.ptr, &x) == HMAP_OK) {
        t.kind = TOKEN_COND_CODE;
        
        t.cond_code = x;
    } else {
        t.kind = TOKEN_IDENT;
        try_else(add_lexical_token(ctx, TOKEN_IDENT), LEX_OK, goto _error);
        return LEX_OK;
    }
    

   

    try_else(vec_push(&ctx->out, &t), VEC_OK, goto _error);
    vec_empty(&ctx->buffer);

    return LEX_OK;

_error:
    
    return LEX_ERR;
}


static enum lexer_result read_ascii(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    next(ctx);

    
    while(ctx->c != EOF && ctx->c != '"') {
        if (!is_printable_char(ctx->c)) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "'%c' is not a valid ascii character.", ctx->c);
            return LEX_ERR;
        }
        try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, return LEX_ERR);
        next(ctx);
    }
    if (ctx->c != '"') {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Unterminated ascii literal.");
        return LEX_ERR;
    }
    next(ctx);

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, return LEX_ERR);

    try_else(add_lexical_token(ctx, TOKEN_ASCII), LEX_OK, return LEX_ERR);
    
    return LEX_OK;



}



static enum lexer_result validate_number(struct lexer_context *ctx, const char *str, int32_t *n) {
    int base = 10;
    size_t len = strlen(str);

    const char *end = &str[len-1];
    if (*end == 'd') {
    } else if (*end == 'x') {
        base = 16;
    } else if (*end == 'b') {
        base = 2;
    } else {
        end++;
    }
    char *endptr;
    errno = 0;
    long res = strtol(str, &endptr, base);

    if (endptr != end) {
        strcpy(ctx->error_msg, "Invalid number litaral");
        return LEX_ERR;
    }
    if (errno == ERANGE || res < INT32_MIN || res > INT32_MAX) {
        strcpy(ctx->error_msg, "Number literal overflow");
        return LEX_ERR;
    }

    
    *n = res;
    vec_empty(&ctx->buffer);
    return LEX_OK;
}


static enum lexer_result read_number(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    
    
    try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, return LEX_ERR);
    next(ctx);

    while (ctx->c != EOF && is_hex_digit_char(ctx->c)) {
        try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, return LEX_ERR);
        next(ctx);
    }

    if (ctx->c != EOF && is_radix_char(ctx->c)) {
        try_else(vec_push(&ctx->buffer, &ctx->c), VEC_OK, return LEX_ERR);
        next(ctx);
    }

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, return LEX_ERR;);

    int32_t n;
    try_else(validate_number(ctx, ctx->buffer.ptr, &n), LEX_OK, return LEX_ERR);
    
    struct token t = {
        //.line = ctx->line,
        .col = ctx->start_col,
        .kind = TOKEN_NUM,
        .number = n
    };
    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR;);



    return LEX_OK;


}



static enum lexer_result make_hash_maps(struct lexer_context *ctx) {

    bool _dir_map = false;
    bool _instr_map = false;
    //_map = false;
    bool _reg_map = false;
    bool _sys_reg_map = false;
    bool _addr_reg_map = false;
    bool _port_map = false;
    bool _data_unit_map = false;
    bool _cond_code_map = false;
    bool _punct_map = false;

    /* INSTRUCTIONS */

    try_else(hashmap_init(&ctx->instr_map, 128), HMAP_OK, goto _error);
    _instr_map = true;
    #define X(u, l, a1, a2, a3) try_else(hashmap_add(&ctx->instr_map, #l, INSTR_##u), HMAP_OK, goto _error);
    #include "resources/instructions.def"
    #undef X

    /* MACROS

    try_else(hashmap_init(&ctx->macro_map, 16), HMAP_OK, goto _error);
    _macro_map = true;

    try_else(hashmap_add(&ctx->macro_map, "!b", MACRO_B), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->macro_map, "!bl", MACRO_BL), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->macro_map, "!movl", MACRO_MOVL), HMAP_OK, goto _error);
    */
    /* DIRECTIVES */

    try_else(hashmap_init(&ctx->dir_map, 16), HMAP_OK, goto _error);
    _dir_map = true;

    #define X(u, l) try_else(hashmap_add(&ctx->dir_map, #l, DIR_##u), HMAP_OK, goto _error);
    #include "resources/directives.def"
    #undef X

    /* REGISTERS */

    try_else(hashmap_init(&ctx->reg_map, 32), HMAP_OK, goto _error);
    _reg_map = true;

    #define X(u, l) try_else(hashmap_add(&ctx->reg_map, #l, REG_##u), HMAP_OK, goto _error);
    #include "resources/registers.def"
    #undef X



    try_else(hashmap_init(&ctx->cond_code_map, 32), HMAP_OK, goto _error);
    _cond_code_map = true;
    #define X(u, l) try_else(hashmap_add(&ctx->cond_code_map, #l, COND_##u), HMAP_OK, goto _error);
    #include "resources/cond_code.def"
    #undef X

    try_else(hashmap_init(&ctx->addr_reg_map, 32), HMAP_OK, goto _error);
    _addr_reg_map = true;
    #define X(u, l) try_else(hashmap_add(&ctx->addr_reg_map, #l, ADDR_REG_##u), HMAP_OK, goto _error);
    #include "resources/addr_registers.def"
    #undef X


    try_else(hashmap_init(&ctx->sys_reg_map, 16), HMAP_OK, goto _error);
    _sys_reg_map = true;
    #define X(u, l) try_else(hashmap_add(&ctx->sys_reg_map, #l, SYS_REG_##u), HMAP_OK, goto _error);
    #include "resources/sys_registers.def"
    #undef X


    try_else(hashmap_init(&ctx->port_map, 16), HMAP_OK, goto _error);
    _port_map = true;
    #define X(u, l) try_else(hashmap_add(&ctx->port_map, #l, PORT_##u), HMAP_OK, goto _error);
    #include "resources/ports.def"
    #undef X

    try_else(hashmap_init(&ctx->punct_map, 16), HMAP_OK, goto _error);
    _punct_map = true;
    #define X(u, l) try_else(hashmap_add(&ctx->punct_map, l, PUNCT_##u), HMAP_OK, goto _error);
    #include "resources/punctuation.def"
    #undef X

    try_else(hashmap_init(&ctx->data_unit_map, 4), HMAP_OK, goto _error);
    _data_unit_map = true;
    #define X(u, l) try_else(hashmap_add(&ctx->data_unit_map, #l, DATA_##u), HMAP_OK, goto _error);
    #include "resources/data_units.def"
    #undef X
    
    

    return LEX_OK;

_error:

    if (_punct_map) hashmap_deinit(&ctx->punct_map);
    if (_cond_code_map) hashmap_deinit(&ctx->cond_code_map);
    if (_data_unit_map) hashmap_deinit(&ctx->data_unit_map);
    if (_port_map) hashmap_deinit(&ctx->port_map);
    if (_addr_reg_map) hashmap_deinit(&ctx->addr_reg_map);
    if (_sys_reg_map) hashmap_deinit(&ctx->sys_reg_map);
    if (_reg_map) hashmap_deinit(&ctx->reg_map);
    //if (_macro_map) hashmap_deinit(&ctx->macro_map);
    if (_instr_map) hashmap_deinit(&ctx->instr_map);
    if (_dir_map) hashmap_deinit(&ctx->dir_map);

    return LEX_ERR;
}

enum lexer_result tokenise(struct source_file *in, struct token **out, uint32_t *out_n, struct compiler_error *error) {

    error->file = in->filename;
    struct lexer_context ctx = {
        .in = in->file,
    

        .line = 1,
        .col = 1,
        .start_col = 1,

        
        ._buffer = false
    }; 
    strcpy(ctx.error_msg, "Unknown error.");

    bool _out = false;
    
    try_else(make_hash_maps(&ctx), LEX_OK, goto _error);
    

    try_else(vec_init(&ctx.buffer, 6, sizeof(char)), VEC_OK, goto _error);
    ctx._buffer = true;
    try_else(vec_init(&ctx.out, 100, sizeof(struct token)), VEC_OK, goto _error);
    _out = true;


    
    next(&ctx);
    while (ctx.c != EOF) {
        
        if (ctx.c == ';') {
            try_else(read_comment(&ctx), LEX_OK, goto _error);
            
        } else if (ctx.c == '\n') {
            try_else(read_new_line(&ctx), LEX_OK, goto _error);
            
        } else if (is_whitespace_char(ctx.c)) {
            try_else(read_whitespace(&ctx), LEX_OK, goto _error);
            
        } else if (is_punctuation_char(ctx.c)) {
            try_else(read_punctuation(&ctx, ctx.c), LEX_OK, goto _error);
            
        } else if (ctx.c == '.') {
            try_else(read_directive(&ctx), LEX_OK, goto _error);
            
        } /*else if (ctx.c == '!') {
            try_else(read_macro(&ctx), LEX_OK, goto _error);
            
        }*/
        else if (ctx.c == '"') {
            try_else(read_ascii(&ctx), LEX_OK, goto _error);
            
        } else if (is_digit_char(ctx.c) || ctx.c == '-') {
            try_else(read_number(&ctx), LEX_OK, goto _error);
            
        } else if (is_word_start_char(ctx.c)) {
            try_else(read_word(&ctx), LEX_OK, goto _error);
            
        } else {
            snprintf(ctx.error_msg, ERR_MSG_LEN, "'%c' is not a valid character", ctx.c);
            goto _error;
        }
    }

    
    struct token eof = {
        .kind = TOKEN_EOF,
        //.line = ctx.line,
        .col = ctx.col,
        .lexeme = NULL
    };

    try_else(vec_push(&ctx.out, &eof), VEC_OK, goto _error);

    if (ctx.out.length > UINT32_MAX) {
        snprintf(ctx.error_msg, ERR_MSG_LEN, "File is too large.");
        goto _error;
    }

    *out = ctx.out.ptr;
    *out_n = ctx.out.length;

    vec_deinit(&ctx.buffer, NULL);

    hashmap_deinit(&ctx.punct_map);
    hashmap_deinit(&ctx.cond_code_map);
    hashmap_deinit(&ctx.data_unit_map);
    hashmap_deinit(&ctx.port_map);
    hashmap_deinit(&ctx.addr_reg_map);
    hashmap_deinit(&ctx.sys_reg_map);
    hashmap_deinit(&ctx.reg_map);
    //hashmap_deinit(&ctx.macro_map);
    hashmap_deinit(&ctx.instr_map);
    hashmap_deinit(&ctx.dir_map);


    return LEX_OK;

_error:
    
    *out = NULL;
    *out_n = 0;
    if (ctx._buffer) vec_deinit(&ctx.buffer, NULL);
    if (_out) vec_deinit(&ctx.out, &_token_deinit);


    
    error->line = ctx.line;
    error->col = ctx.start_col;
    error->kind = CERROR_LEXER;
    strcpy(error->msg, ctx.error_msg);

    hashmap_deinit(&ctx.punct_map);
    hashmap_deinit(&ctx.cond_code_map);
    hashmap_deinit(&ctx.data_unit_map);
    hashmap_deinit(&ctx.port_map);
    hashmap_deinit(&ctx.addr_reg_map);
    hashmap_deinit(&ctx.sys_reg_map);
    hashmap_deinit(&ctx.reg_map);
    //hashmap_deinit(&ctx.macro_map);
    hashmap_deinit(&ctx.instr_map);
    hashmap_deinit(&ctx.dir_map);


    return LEX_ERR;
}




