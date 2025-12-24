
#ifndef __LEXER_ERROR_HEADER
#define __LEXER_ERROR_HEADER

enum lexer_error_kind {
    LEX_SUCCESS
};

struct lexer_error {
    enum lexer_error_kind kind;
    unsigned long line;
};

#endif
