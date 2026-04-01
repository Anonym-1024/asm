

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "error/compiler_error.h"
#include "lexer/lexer.h"
#include "shared/sema_output.h"
#include "shared/token.h"
#include "shared/ast.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "codegen/codegen.h"








int main(int argc, char **argv) {

    if (argc < 2) {
        printf("\033[31mNo input file.\n");
        return -1;
    }
    if (argc > 2) {
        printf("\033[31mToo many arguments.\n");
        return -1;
    }

    FILE *input;
    if (strcmp("-s", argv[1]) == 0) {
        input = stdin;
    } else {
        input = fopen(argv[1], "r");
    }
    if (input == NULL) {
        printf("\033[31mCould not open file '%.100s'.\n", argv[1]);
        return -1;
    }




    struct compiler_error error;

    struct token *tokens;
    uint32_t tokens_n;

    enum lexer_result lex_res = tokenise(input, &tokens, &tokens_n, &error);

    if (lex_res == LEX_ERR) {
        print_compiler_error(stderr, &error, argv[1]);
        return -1;
    }



    fclose(input);



    struct ast_file root;

    enum parser_result pars_res = parse(tokens, tokens_n, &root, &error);

    if (pars_res == PARSER_ERR) {
        print_compiler_error(stderr, &error, argv[1]);
        for (uint32_t i = 0; i < tokens_n; i++) {
            token_deinit(&tokens[i]);
        }
        return -1;
    }

    





    //uint32_t start_addr;
    struct sema_output o;
    enum sema_result sema_res = perform_semantic_analysis(&root, &o, &error);

    if (sema_res == SEMA_ERR) {
        print_compiler_error(stderr, &error, argv[1]);
        for (uint32_t i = 0; i < tokens_n; i++) {
            token_deinit(&tokens[i]);
        }
        ast_file_deinit(&root);
        return -1;
    }


    FILE *output = fopen("out.bin", "wb");

    enum codegen_result kk = generate_object_file(&root, &o, output, &error);
    if (kk != CODEGEN_OK) {
        print_compiler_error(stderr, &error, argv[1]);
        
    }
    

    //generate_binary(&root, start_addr, output);



    for (uint32_t i = 0; i < tokens_n; i++) {
        token_deinit(&tokens[i]);
    }
    free(tokens);

    ast_file_deinit(&root);

    fclose(output);

    return 0;
}
