

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "error/compiler_error.h"
#include "compilation/compilation.h"
#include "linker/linker.h"








int main(int argc, const char **argv) {

    const char *output = NULL;
    const char *c_input = NULL;
    const char **l_input = NULL;
    int l_input_len = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i+1 >= argc) {
                printf("\033[31mExpected output file name after '-o'.");
                return -1;
            }

            output = argv[i+1];
        }

        if (strcmp(argv[i], "-c") == 0) {
            if (i+1 >= argc) {
                printf("\033[31mExpected input file name after '-c'.");
                return -1;
            }

            c_input = argv[i+1];
        }

        if (strcmp(argv[i], "-l") == 0) {

            for (int j = i+1; j < argc && argv[j][0] != '-'; j++) {
                l_input_len++;
            }

            if (l_input_len == 0) {
                printf("\033[31mExpected input files after '-l'.");
                return -1;
            }
            l_input = &argv[i+1];

        }
        
    }

    if (c_input != NULL && l_input != NULL) {
        printf("\033[31mConnot use both '-l' and '-c' at the same time.");
        return -1;
    }
    
    struct compiler_error error;
    if (c_input != NULL) {
        enum compilation_result c_res;
        c_res = compile_source_file(c_input, output, &error);
        if (c_res != COMP_OK) {
            print_compiler_error(stderr, &error);
            return -1;
        }
        
    } else if (l_input != NULL) {
        enum linker_result l_res;
        l_res = link_object_files(l_input, l_input_len, output, &error);
        if (l_res != LINK_OK) {
            print_compiler_error(stderr, &error);
            return -1;
        }
        
    } else {
        printf("\033[31mUse '-c <file>' or '-l <files>'.");
        return -1;
    }

    return 0;
}
