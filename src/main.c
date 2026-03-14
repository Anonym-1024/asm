

#include "lexer/lexer.h"
#include "libs/vector/vector.h"
#include "libs/error_handling.h"
#include "libs/utilities/utilities.h"
#include "error/compiler_error.h"
#include <assert.h>
#include "parser/parser.h"
#include <unistd.h>
#include "libs/hashmap/hashmap.h"
#include "libs/arena/arena.h"
#include "sema/sema.h"
#include <time.h>

int main(void) {



    char *dd = "resources/example.asm";
    
    

    

    

    FILE *in = fopen(dd, "r");

    
    struct token *out;
    uint32_t cc;
    
    struct compiler_error err;

    
    enum lexer_result c = tokenise(in, &out, &cc, &err);
    fclose(in);
    printf("Lexer done: %d", cc);
    

    if (c == LEX_ERR) {
        
        print_compiler_error(stdout, &err);
        
   
        
        


        return 0;
    }
    

    
    struct ast_file file;
    enum parser_result ss = parse(out, cc, &file, &err);
    
    if (ss == PARSER_OK) {
        
        
        
        printf("Hura");
        
        
    } else {
        print_compiler_error(stdout, &err);
        return 0;
    }
    
    
    if (perform_semantic_analysis(&file, &err) != SEMA_OK) {
        print_compiler_error(stdout, &err);
    }
    
    for (uint32_t i = 0; i < cc; i++) {
        token_deinit(&out[i]);

    }
    free(out);
    
    
    
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
