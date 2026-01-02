

#include "lexer/lexer.h"
#include "libs/vector/vector.h"
#include "libs/error_handling.h"
#include "libs/utilities/utilities.h"
#include "parser/parser.h"





int main(void) {
    
    FILE *f = fopen("resources/example.asm", "r");

    

    off_t s = get_file_size("resources/example.asm");

    char *in = malloc(sizeof(char) * s);
    fread(in, sizeof(char), s, f);

    struct vector out;

    struct lexer_error err;

    
    fflush(stdout);
    if (tokenise(in, s, &out, &err) == LEX_OK) {
        for (size_t i = 0; i < out.length; i++) {
            struct token t;
            vec_get(&out, &t, i);
            char *d;
            token_desc(&t, &d);
            printf("\033[32m%s\n", d);
        }
    } else {
        printf("\033[31m%s", lexer_error_desc(&err));
        return 0;
    }

    struct cst_node j;
    struct parser_error k;
    parse(out.ptr, out.length, &j, &k);
    
    
    
    printf("%s\n", parser_error_desc(&k));
    print_cst_node(stdout, &j, 0);
    return 0;
}
