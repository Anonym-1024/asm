#ifndef __TOKEN_HEADER__
#define __TOKEN_HEADER__


#include <stdint.h>

enum token_kind {
    TOKEN_DIR,
    TOKEN_DATA_UNIT,
    TOKEN_INSTR,
    TOKEN_MACRO,
    TOKEN_COND_CODE,
    TOKEN_REG,
    TOKEN_SYS_REG,
    TOKEN_PORT,
    TOKEN_ADDR_REG,
    TOKEN_NUM,
    TOKEN_ASCII,
    TOKEN_IDENT,
    TOKEN_PUNCT,
    TOKEN_EOF
};


enum instruction_token {
    #define X(u, l, a1, a2, a3) INSTR_##u,
    #include "resources/instructions.def"
    #undef X
};


enum directive_token {
    #define X(u, l) DIR_##u,
    #include "resources/directives.def"
    #undef X
};

enum register_token {
    #define X(u, l) REG_##u,
    #include "resources/registers.def"
    #undef X
};

enum sys_register_token {
    #define X(u, l) SYS_REG_##u,
    #include "resources/sys_registers.def"
    #undef X
};

enum addr_register_token {
    #define X(u, l) ADDR_REG_##u,
    #include "resources/addr_registers.def"
    #undef X
};

enum port_token {
    #define X(u, l) PORT_##u,
    #include "resources/ports.def"
    #undef X
};

enum data_unit_token {
    #define X(u, l) DATA_##u,
    #include "resources/data_units.def"
    #undef X
};

enum cond_code_token {
    #define X(u, l, n) COND_##u = n,
    #include "resources/cond_code.def"
    #undef X
};

enum punctuation_token {
    #define X(u, l) PUNCT_##u,
    #include "resources/punctuation.def"
    #undef X
};


struct token {
    uint8_t kind;
    uint16_t col;
    union {
        char *lexeme;
        int32_t number;
        enum instruction_token instr;
        enum directive_token dir;
        enum data_unit_token data_unit;
        enum cond_code_token cond_code;
        enum register_token reg;
        enum sys_register_token sys_reg;
        enum addr_register_token addr_reg;
        enum port_token port;
        enum punctuation_token punct;
    };

};

void token_deinit(struct token *ptr);
void _token_deinit(void *ptr);


#endif
