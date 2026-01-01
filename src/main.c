

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
        for (int i = 0; i < out.length; i++) {
            struct token t;
            vec_get(&out, &t, i);
            printf("\033[32m%s\n", token_desc(&t));
        }
    } else {
        printf("\033[31m%s", lexer_error_desc(&err));
    }

    struct cst_node j;
    struct parser_error k;
    parse(out.ptr, out.length, &j, &k);
    
    
    printf("%s", parser_error_desc(&k));
    return 0;
}
