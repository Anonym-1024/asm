

#include "lexer/lexer.h"
#include "libs/vector/vector.h"
#include "libs/error_handling.h"
#include "libs/utilities/utilities.h"
#include "error/compiler_error.h"
#include <assert.h>
#include "parser/parser.h"


int main(void) {

    

    FILE *f = fopen("resources/example.asm", "r");

    

    off_t s = get_file_size("resources/example.asm");

    char *in = malloc(sizeof(char) * s);
    fread(in, sizeof(char), s, f);

    
    struct vector out;
    
    struct compiler_error err;

    
    enum lexer_result c = tokenise(in, s, &out, &err);
    

    if (c == LEX_ERR) {
        
        print_compiler_error(stdout, &err);
        compiler_error_deinit(&err);
   
        fclose(f);
        


        return 0;
    }

    
    struct ast_file file;
    enum parser_result ss = parse(out.ptr, out.length, &file, &err);

    if (ss == PARSER_OK) {
        
        ast_file_deinit(&file);
    } else {
        print_compiler_error(stdout, &err);
        compiler_error_deinit(&err);
    }
    
    vec_deinit(&out, _token_deinit);

    /*struct cst_node pout;
    struct parser_error perr;
    enum parser_result pres = parse(out.ptr, out.length, &pout, &perr);
    if (pres == PARSER_OK) {
        print_cst_node(stdout, &pout, 0);
    } else {
        char *desc;
        parser_error_desc(&perr, &desc);
        printf("%s", desc);
        free(desc);
    }
    

    free(perr.msg);
    cst_node_deinit(&pout);
    free(in);
    vec_deinit(&out, &_token_deinit);
    fclose(f);
    */
    return 0;
}
