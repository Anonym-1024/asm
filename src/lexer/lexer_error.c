
#include "lexer_error.h"


char *lexer_error_desc(struct lexer_error *err) {

    const char *msg;

    switch (err->kind) {
        case LEX_UNKNOWN_ERR:
        msg = "Unknown error.";
        break;

        case LEX_MEM_ERR:
        msg = "Memory error.";
        break;
        
        case LEX_INV_DIR:
        msg = "Invalid directive.";
        break;
        
        case LEX_INV_MACRO:
        msg = "Invalid macro.";
        break;
        
        case LEX_INV_CHAR:
        msg = "Invalid character.";
        break;
        
        case LEX_INV_ASCII_LIT:
        msg = "Invalid ascii literal.";
        break;
        
        case LEX_UNTERM_ASCII_LIT:
        msg = "Unterminated ascii literal.";
        break;
        
        case LEX_INV_NUM:
        msg = "Invalid number literal.";
        break;
    }

    char *out;
    asprintf(&out, "LEXER ERROR [%ld:%ld]: %s", err->line, err->col, msg);

    return out;
    
}