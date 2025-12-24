
#include "lexer.h"


struct lexer_context {
    const char *in;
    unsigned long n;
    unsigned long index;
    struct token *out;
    unsigned long line;
};



bool is_word_start_char(char c) {
    return (c >= 'A' && c <= 'z') || (c == '_');
}

bool is_word_char(char c) {
    return (c >= 'A' && c <= 'z') || (c == '_') || (c >= '0' && c <= '9');
}

enum token_kind get_word_token_kind(const char *word) {
    
}








struct lexer_error tokenise(const char *in, unsigned long n, struct token *out) {

    enum lexer_error_kind err = LEX_SUCCESS;
    
    struct lexer_context ctx = {
        .in = in,
        .n = n,
        .index = 0,
        .out = out,
        .line = 0
    };


    while (ctx.index < ctx.n) {
        
    }
    


    

    struct lexer_error _err = {
        .kind = err,
        .line = ctx.line
    };
    return _err;
}
