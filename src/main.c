

#include "lexer/lexer.h"
#include "libs/vector/vector.h"
#include "libs/error_handling.h"
#include "libs/utilities/utilities.h"
#include "parser/parser.h"
#include <assert.h>
#include "ast/ast.h"




int main(void) {
    FILE *f = fopen("resources/example copy.asm", "r");

    

    off_t s = get_file_size("resources/example copy.asm");

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

    for (size_t i = 0; i<out.length; i++) {
        get_token_desc(vec_get_ptr(&out, i), &desc);
        printf("%s\n", desc);
        free(desc);
    }
    

    free(in);
    vec_deinit(&out, &_token_deinit);
    fclose(f);

    return 0;
}
