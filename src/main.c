

#include "lexer/lexer.h"
#include "libs/vector/vector.h"
#include "libs/error_handling.h"
#include "libs/utilities/utilities.h"
#include "error/compiler_error.h"
#include <assert.h>
#include "parser/parser.h"
#include <unistd.h>
#include "libs/hashmap/hashmap.h"

int main(void) {


    struct hashmap map;
    hashmap_init(&map, 2, sizeof(int), NULL);
    int c = 8;
    hashmap_add(&map, "hodnota", &c);
    c = 11;
    hashmap_add(&map, "klic", &c);

    c = 13;
    hashmap_add(&map, "klic2", &c);

    c = 14;
    hashmap_add(&map, "klic3", &c);

    c = 893;
    hashmap_add(&map, "klic4", &c);

    c = 13232;
    hashmap_add(&map, "klic5", &c);

    c = 937;
    hashmap_add(&map, "klic6", &c);

    c = 113;
    hashmap_add(&map, "klic5", &c);


    hashmap_get(&map, "klic5", &c);
    printf("hodnota: %d", c);
    if (hashmap_find(&map, "klic") == HMAP_FOUND) {
        printf("found");
    }


    
    /*
    FILE *f = fopen("resources/example copy.asm", "r");

    

    off_t s = get_file_size("resources/example copy.asm");

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
        
        //printf("%s", vec_get_ptr_t(&vec_get_ptr_t(&file.sections, 0, struct ast_section)->data_section.data_stmts, 0, struct ast_data_stmt)->label_stmt.ident.lexeme);
        
        ast_file_deinit(&file);
    } else {
        print_compiler_error(stdout, &err);
        compiler_error_deinit(&err);
    }
    
    vec_deinit(&out, &_token_deinit);
    */
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
