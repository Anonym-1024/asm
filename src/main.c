

#include "lexer/lexer.h"
#include "libs/vector/vector.h"
#include "libs/error_handling.h"
#include "libs/utilities/utilities.h"
#include "parser/parser.h"
#include <assert.h>
#include "ast/ast.h"


int main(void) {

    

    FILE *f = fopen("resources/example.asm", "r");

    

    off_t s = get_file_size("resources/example.asm");

    char *in = malloc(sizeof(char) * s);
    fread(in, sizeof(char), s, f);

    
    struct vector out;
    
    struct lexer_error err;

    
    enum lexer_result c = tokenise(in, s, &out, &err);
    char *desc;

    if (c == LEX_ERR) {
        
        lexer_error_desc(&err, &desc);
        printf("%s", desc);
        free(in);
   
        fclose(f);
        free(desc);


        return 0;
    }

    struct cst_node pout;
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

    return 0;
}
