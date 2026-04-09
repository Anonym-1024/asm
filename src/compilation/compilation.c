
#include "compilation.h"
#include "stdlib.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "codegen/codegen.h"
#include "libs/error_handling.h"
#include "error/compiler_error.h"










enum compilation_result compile_source_file(const char *in, const char *out, struct compiler_error *err) {

    strcpy(err->msg, "Unknown error.");
    err->file = NULL;
    err->line = 0;
    err->col = 0;
    err->kind = CERROR_UNKNOWN;

    bool _src = false;
    bool _tokens = false;
    bool _ast_root = false;
    bool _sema_output = false;
    bool _out = false;

    struct source_file src;
    src.file = fopen(in, "r");
    if (src.file == NULL) {
        goto _error;
    }
    _src = true;
    src.filename = in;

    struct token *tokens;
    uint32_t tokens_n;
    struct ast_file ast_root;
    struct sema_output sema_out;




    try_else(tokenise(&src, &tokens, &tokens_n, err), LEX_OK, goto _error);
    _tokens = true;

    try_else(parse(tokens, tokens_n, src.filename, &ast_root, err), PARSER_OK, goto _error);
    _ast_root = true;

    try_else(perform_semantic_analysis(&ast_root, &sema_out, err), SEMA_OK, goto _error);
    _sema_output = true;


    if (out == NULL) {
        out = malloc(sizeof(char) * (strlen(src.filename) + 3));
        sprintf((char*)out, "%s.o", src.filename);
        _out = true;
    }


    try_else(generate_object_file(&ast_root, &sema_out, out, err), CODEGEN_OK, goto _error);

    if (_out) {
        free((char*)out);
        _out = false;
    }

    for (uint32_t i = 0; i < tokens_n; i++) {
        token_deinit(&tokens[i]);
    }
    free(tokens);

    ast_file_deinit(&ast_root);

    sema_output_deinit(&sema_out);


    fclose(src.file);



    return COMP_OK;

_error:

    if (_src) {
        fclose(src.file);
    }

    if (_tokens) {
        for (uint32_t i = 0; i < tokens_n; i++) {
            token_deinit(&tokens[i]);
        }
    }

    if (_ast_root) {
        ast_file_deinit(&ast_root);
    }

    if (_sema_output) {
        sema_output_deinit(&sema_out);
    }

    if (_out) {
        free((char*)out);
    }

    return COMP_ERR;
}
