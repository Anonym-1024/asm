
#include "lexer.h"

#include <ctype.h>
#include "libs/vector/vector.h"
#include "libs/hashmap/hashmap.h"
#include "libs/error_handling.h"


struct lexer_context {
    const char *in; //! Reference
    uint32_t n;
    uint32_t index;
    struct vector out; //! Owned
    
    uint32_t line;
    uint32_t col;
    uint32_t start_col;

    struct vector buffer; //! Owned
    bool _buffer;

    char error_msg[ERR_MSG_LEN];

    struct hashmap dir_map;
    struct hashmap instr_map;
    struct hashmap macro_map;
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
    return (c == ',') || (c == '{') || (c == '}') || (c == ':') || (c == '(') || (c == ')') || (c == '#');
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


enum lexer_result add_lexical_token(struct lexer_context *ctx, enum token_kind kind) {
    

    struct token t;
    t.kind = kind;
    t.line = ctx->line;
    t.col = ctx->start_col;

    t.lexeme = ctx->buffer.ptr;
    

    
    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR);
    ctx->_buffer = false;
    try_else(vec_init(&ctx->buffer, 6, sizeof(char)), VEC_OK, return LEX_ERR);
    ctx->_buffer = true;

    return LEX_OK;




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

    
    struct token t = {
        .kind = TOKEN_PUNCT,
        .line = ctx->line,
        .col = ctx->start_col,
        .punct = PUNCT_NEWLINE
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR);
    
    pop_char(ctx);
    ctx->col = 1;
    ctx->line += 1;
    
    return LEX_OK;


}


static enum lexer_result read_whitespace(struct lexer_context *ctx) {
    pop_char(ctx);
    return LEX_OK;
}


static enum lexer_result read_punctuation(struct lexer_context *ctx, char c) {
    ctx->start_col = ctx->col;

    try_else(vec_push(&ctx->buffer, &c), VEC_OK, return LEX_ERR);
    pop_char(ctx);

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, return LEX_ERR);

    
    enum punctuation_token p;
    hashmap_get(&ctx->punct_map, ctx->buffer.ptr, &p);

    struct token t = {
        .kind = TOKEN_PUNCT,
        .line = ctx->line,
        .col = ctx->start_col,
        .punct = p
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, return LEX_ERR);
    vec_empty(&ctx->buffer);
    return LEX_OK;



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


    uint64_t _t;
    enum directive_token d;
    if (hashmap_get(&ctx->dir_map, ctx->buffer.ptr, &d) != HMAP_OK) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "'%s' is not a valid directive.", (char*)ctx->buffer.ptr);
        goto _error;
    }
     

    struct token t = {
        .kind = TOKEN_DIR,
        .line = ctx->line,
        .col = ctx->start_col,
        .dir = d
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, goto _error);
    vec_empty(&ctx->buffer);

    return LEX_OK;

_error:
   
    return LEX_ERR;
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

    
    enum macro_token m;
    if (hashmap_get(&ctx->macro_map, ctx->buffer.ptr, &m) != HMAP_OK) {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "'%s' is not a valid macro.", (char*)ctx->buffer.ptr);
        goto _error;
    }
    
    
    struct token t = {
        .kind = TOKEN_MACRO,
        .line = ctx->line,
        .col = ctx->start_col,
        .macro = m
    };

    try_else(vec_push(&ctx->out, &t), VEC_OK, goto _error);
    vec_empty(&ctx->buffer);

    return LEX_OK;

_error:
    
    return LEX_ERR;
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

    

    struct token t = {
        .line = ctx->line,
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
    pop_char(ctx);

    char c;
    while(get_char(ctx, &c) == true && c != '"') {
        if (!is_printable_char(c)) {
            snprintf(ctx->error_msg, ERR_MSG_LEN, "'%c' is not a valid ascii character.", c);
            return LEX_ERR;
        }
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, return LEX_ERR);
        pop_char(ctx);
    }
    if (c != '"') {
        snprintf(ctx->error_msg, ERR_MSG_LEN, "Unterminated ascii literal.");
        return LEX_ERR;
    }
    pop_char(ctx);

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
    return LEX_OK;
}


static enum lexer_result read_number(struct lexer_context *ctx) {
    ctx->start_col = ctx->col;
    char c;
    get_char(ctx, &c);
    try_else(vec_push(&ctx->buffer, &c), VEC_OK, return LEX_ERR);
    pop_char(ctx);

    while (get_char(ctx, &c) == true && is_hex_digit_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, return LEX_ERR);
        pop_char(ctx);
    }

    if (get_char(ctx, &c) == true && is_radix_char(c)) {
        try_else(vec_push(&ctx->buffer, &c), VEC_OK, return LEX_ERR);
        pop_char(ctx);
    }

    char term = 0;
    try_else(vec_push(&ctx->buffer, &term), VEC_OK, return LEX_ERR;);

    int32_t n;
    try_else(validate_number(ctx, ctx->buffer.ptr, &n), LEX_OK, return LEX_ERR);

    struct token t = {
        .line = ctx->line,
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
    bool _macro_map = false;
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

    try_else(hashmap_add(&ctx->instr_map, "mov", INSTR_MOV), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "movs", INSTR_MOVS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "mvn", INSTR_MVN), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "mvns", INSTR_MVNS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "srw", INSTR_SRW), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "srr", INSTR_SRR), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "ldr", INSTR_LDR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ldro", INSTR_LDRO), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ldri", INSTR_LDRI), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "str", INSTR_STR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "stro", INSTR_STRO), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "stri", INSTR_STRI), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "add", INSTR_ADD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "adds", INSTR_ADDS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "addc", INSTR_ADDC), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "addcs", INSTR_ADDCS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "sub", INSTR_SUB), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "subs", INSTR_SUBS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "subc", INSTR_SUBC), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "subcs", INSTR_SUBCS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "and", INSTR_AND), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ands", INSTR_ANDS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "or", INSTR_OR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ors", INSTR_ORS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "eor", INSTR_EOR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "eors", INSTR_EORS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "lsl", INSTR_LSL), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "lsls", INSTR_LSLS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "lsr", INSTR_LSR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "lsrs", INSTR_LSRS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "asr", INSTR_ASR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "asrs", INSTR_ASRS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "cls", INSTR_CLS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "csls", INSTR_CSLS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "csr", INSTR_CSR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "csrs", INSTR_CSRS), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "cmn", INSTR_CMN), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "addcd", INSTR_ADDCD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "cmp", INSTR_CMP), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "subcd", INSTR_SUBCD), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "andd", INSTR_ANDD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ord", INSTR_ORD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "eord", INSTR_EORD), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "lsld", INSTR_LSLD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "lsrd", INSTR_LSRD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "asrd", INSTR_ASRD), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "csld", INSTR_CSLD), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "csrd", INSTR_CSRD), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "ba", INSTR_BA), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "bal", INSTR_BAL), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "br", INSTR_BR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "brl", INSTR_BRL), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "ptr", INSTR_PTR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ptw", INSTR_PTW), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->instr_map, "ptsr", INSTR_PTSR), HMAP_OK, goto _error);

    try_else(hashmap_add(&ctx->instr_map, "svc", INSTR_SVC), HMAP_OK, goto _error);

    /* MACROS */

    try_else(hashmap_init(&ctx->macro_map, 16), HMAP_OK, goto _error);
    _macro_map = true;

    try_else(hashmap_add(&ctx->macro_map, "!b", MACRO_B), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->macro_map, "!bl", MACRO_BL), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->macro_map, "!movl", MACRO_MOVL), HMAP_OK, goto _error);

    /* DIRECTIVES */

    try_else(hashmap_init(&ctx->dir_map, 16), HMAP_OK, goto _error);
    _dir_map = true;

    try_else(hashmap_add(&ctx->dir_map, ".DATA", DIR_DATA), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->dir_map, ".EXEC", DIR_EXEC), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->dir_map, ".start", DIR_START), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->dir_map, ".l", DIR_L), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->dir_map, ".b", DIR_B), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->dir_map, ".f", DIR_F), HMAP_OK, goto _error);

    /* REGISTERS */

    try_else(hashmap_init(&ctx->reg_map, 32), HMAP_OK, goto _error);
    _reg_map = true;

    try_else(hashmap_add(&ctx->reg_map, "r0", REG_R0), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r1", REG_R1), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r2", REG_R2), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r3", REG_R3), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r4", REG_R4), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r5", REG_R5), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r6", REG_R6), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r7", REG_R7), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r8", REG_R8), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r9", REG_R9), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r10", REG_R10), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r11", REG_R11), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r12", REG_R12), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r13", REG_R13), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r14", REG_R14), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->reg_map, "r15", REG_R15), HMAP_OK, goto _error);



    try_else(hashmap_init(&ctx->cond_code_map, 32), HMAP_OK, goto _error);
    _cond_code_map = true;

    try_else(hashmap_add(&ctx->cond_code_map, "al", COND_AL), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "eq", COND_EQ), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "zs", COND_ZS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "mi", COND_MI), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "vs", COND_VS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "su", COND_SU), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "cc", COND_CC), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "gu", COND_GU), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "ss", COND_SS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "gs", COND_GS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "ne", COND_NE), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "zc", COND_ZC), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "pl", COND_PL), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "vc", COND_VC), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "geu", COND_GEU), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "cs", COND_CS), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "seu", COND_SEU), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "ges", COND_GES), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->cond_code_map, "ses", COND_SES), HMAP_OK, goto _error);

    try_else(hashmap_init(&ctx->addr_reg_map, 32), HMAP_OK, goto _error);
    _addr_reg_map = true;

    try_else(hashmap_add(&ctx->addr_reg_map, "r0a", ADDR_REG_R0A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r1a", ADDR_REG_R1A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r2a", ADDR_REG_R2A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r3a", ADDR_REG_R3A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r4a", ADDR_REG_R4A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r5a", ADDR_REG_R5A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r6a", ADDR_REG_R6A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r7a", ADDR_REG_R7A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r8a", ADDR_REG_R8A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r9a", ADDR_REG_R9A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r10a", ADDR_REG_R10A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r11a", ADDR_REG_R11A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r12a", ADDR_REG_R12A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r13a", ADDR_REG_R13A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r14a", ADDR_REG_R14A), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->addr_reg_map, "r15a", ADDR_REG_R15A), HMAP_OK, goto _error);


    try_else(hashmap_init(&ctx->sys_reg_map, 16), HMAP_OK, goto _error);
    _sys_reg_map = true;

    try_else(hashmap_add(&ctx->sys_reg_map, "pc_b0", SYS_REG_PC_B0), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->sys_reg_map, "pc_b1", SYS_REG_PC_B1), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->sys_reg_map, "psr", SYS_REG_PSR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->sys_reg_map, "intr", SYS_REG_INTR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->sys_reg_map, "pdbr_b0", SYS_REG_PDBR_B0), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->sys_reg_map, "pdbr_b1", SYS_REG_PDBR_B1), HMAP_OK, goto _error)


    try_else(hashmap_init(&ctx->port_map, 16), HMAP_OK, goto _error);
    _port_map = true;

    try_else(hashmap_add(&ctx->port_map, "p0", PORT_P0), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p1", PORT_P1), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p2", PORT_P2), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p3", PORT_P3), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p4", PORT_P4), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p5", PORT_P5), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p6", PORT_P6), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->port_map, "p7", PORT_P7), HMAP_OK, goto _error);

    try_else(hashmap_init(&ctx->punct_map, 16), HMAP_OK, goto _error);
    _punct_map = true;

    try_else(hashmap_add(&ctx->punct_map, ",", PUNCT_COMMA), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->punct_map, ":", PUNCT_SEMICOLON), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->punct_map, "{", PUNCT_LBRACE), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->punct_map, "}", PUNCT_RBRACE), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->punct_map, "(", PUNCT_LPAR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->punct_map, ")", PUNCT_RPAR), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->punct_map, "#", PUNCT_HASH), HMAP_OK, goto _error);

    try_else(hashmap_init(&ctx->data_unit_map, 4), HMAP_OK, goto _error);
    _punct_map = true;

    try_else(hashmap_add(&ctx->data_unit_map, "byte", DATA_BYTE), HMAP_OK, goto _error);
    try_else(hashmap_add(&ctx->data_unit_map, "bytes", DATA_BYTES), HMAP_OK, goto _error);
    
    

    return LEX_OK;

_error:

    if (_punct_map) hashmap_deinit(&ctx->punct_map);
    if (_cond_code_map) hashmap_deinit(&ctx->cond_code_map);
    if (_data_unit_map) hashmap_deinit(&ctx->data_unit_map);
    if (_port_map) hashmap_deinit(&ctx->port_map);
    if (_addr_reg_map) hashmap_deinit(&ctx->addr_reg_map);
    if (_sys_reg_map) hashmap_deinit(&ctx->sys_reg_map);
    if (_reg_map) hashmap_deinit(&ctx->reg_map);
    if (_macro_map) hashmap_deinit(&ctx->macro_map);
    if (_instr_map) hashmap_deinit(&ctx->instr_map);
    if (_dir_map) hashmap_deinit(&ctx->dir_map);

    return LEX_ERR;
}

enum lexer_result tokenise(const char *in, uint32_t n, struct token **out, uint32_t *out_n, struct compiler_error *error) {

    
    struct lexer_context ctx = {
        .in = in,
        .n = n,
        .index = 0,

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
    try_else(vec_init(&ctx.out, 1000, sizeof(struct token)), VEC_OK, goto _error);
    _out = true;


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
            snprintf(ctx.error_msg, ERR_MSG_LEN, "'%c' is not a valid character", c);
            goto _error;
        }
    }

    
    struct token eof = {
        .kind = TOKEN_EOF,
        .line = ctx.line,
        .col = ctx.col,
        .lexeme = NULL
    };

    try_else(vec_push(&ctx.out, &eof), VEC_OK, goto _error);


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
    hashmap_deinit(&ctx.macro_map);
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
    hashmap_deinit(&ctx.macro_map);
    hashmap_deinit(&ctx.instr_map);
    hashmap_deinit(&ctx.dir_map);


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
    if (asprintf(out, "TOKEN [%d:%d]: { Kind: %s; Lexeme: %s }", t->line, t->col, kind, t->lexeme) == -1) {
        goto _error;
    }
    return 0;

_error:
    free(*out);
    return -1;

    
}
