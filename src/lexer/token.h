#ifndef __TOKEN_HEADER__
#define __TOKEN_HEADER__

#include <stdlib.h>

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
    INSTR_MOV, INSTR_MOVS,
    INSTR_MVN,
    INSTR_MVNS,
    INSTR_SRW,
    INSTR_SRR,
    INSTR_LDR,
    INSTR_LDRO,
    INSTR_LDRI,
    INSTR_STR,
	INSTR_STRO,
	INSTR_STRI,
    INSTR_ADD,
	INSTR_ADDS,
	INSTR_ADDC,
	INSTR_ADDCS,
    INSTR_SUB,
	INSTR_SUBS,
	INSTR_SUBC,
	INSTR_SUBCS,
    INSTR_AND,
	INSTR_ANDS,
	INSTR_OR,
	INSTR_ORS,
    INSTR_EOR,
	INSTR_EORS,
    INSTR_LSL,
	INSTR_LSLS,
	INSTR_LSR,
	INSTR_LSRS,
    INSTR_ASR,
	INSTR_ASRS,
    INSTR_CLS,
	INSTR_CSLS,
	INSTR_CSR,
	INSTR_CSRS,
    INSTR_CMN,
	INSTR_ADDCD,
	INSTR_CMP,
	INSTR_SUBCD,
    INSTR_ANDD,
	INSTR_ORD,
	INSTR_EORD,
    INSTR_LSLD,
	INSTR_LSRD,
	INSTR_ASRD,
    INSTR_CSLD,
	INSTR_CSRD,
    INSTR_BA,
	INSTR_BAL,
	INSTR_BR,
	INSTR_BRL,
    INSTR_PTR,
	INSTR_PTW,
	INSTR_PTSR,
    INSTR_SVC
};

enum macro_token {
    MACRO_B,
    MACRO_BL,
    MACRO_MOVL
};

enum directive_token {
    DIR_DATA,
    DIR_EXEC,
    DIR_START,
    DIR_L,
    DIR_B,
    DIR_F
};

enum register_token {
    REG_R0,
	REG_R1,
	REG_R2,
	REG_R3,
    REG_R4,
	REG_R5,
	REG_R6,
	REG_R7,
    REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
    REG_R12,
	REG_R13,
	REG_R14,
	REG_R15
};

enum sys_register_token {
    SYS_REG_PC_B0,
    SYS_REG_PC_B1,
    SYS_REG_PSR,
    SYS_REG_INTR,
    SYS_REG_PDBR_B0,
    SYS_REG_PDBR_B1
};

enum addr_register_token {
    ADDR_REG_R0A,
	ADDR_REG_R1A,
	ADDR_REG_R2A,
	ADDR_REG_R3A,
    ADDR_REG_R4A,
	ADDR_REG_R5A,
	ADDR_REG_R6A,
	ADDR_REG_R7A,
    ADDR_REG_R8A,
	ADDR_REG_R9A,
	ADDR_REG_R10A,
	ADDR_REG_R11A,
    ADDR_REG_R12A,
	ADDR_REG_R13A,
	ADDR_REG_R14A,
	ADDR_REG_R15A
};

enum port_token {
    PORT_P0,
	PORT_P1,
	PORT_P2,
	PORT_P3,
    PORT_P4,
	PORT_P5,
	PORT_P6,
	PORT_P7
};

enum data_unit_token {
    DATA_BYTE,
    DATA_BYTES
};

enum cond_code_token {
    COND_AL,
	COND_EQ,
	COND_ZS,
	COND_MI,
    COND_VS,
	COND_SU,
	COND_CC,
	COND_GU,
    COND_SS,
	COND_GS,
	COND_NE,
	COND_ZC,
    COND_PL,
	COND_VC,
	COND_GEU,
	COND_CS,
    COND_SEU,
	COND_GES,
	COND_SES
};

enum punctuation_token {
    PUNCT_NEWLINE,
    PUNCT_COMMA,
    PUNCT_LPAR,
    PUNCT_RPAR,
    PUNCT_LBRACE,
    PUNCT_RBRACE,
    PUNCT_HASH,
    PUNCT_SEMICOLON
};

struct token {
    enum token_kind kind;
    size_t line; 
    size_t col;
    union {
        char *lexeme;
        enum instruction_token instr;
        enum macro_token macro;
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
