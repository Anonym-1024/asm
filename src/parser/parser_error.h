
#ifndef __PARSER_ERROR_HEADER__
#define __PARSER_ERROR_HEADER__ 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


struct parser_error {
    const char *msg;

    size_t line;
    size_t col;
};

char *parser_error_desc(struct parser_error *err);

#endif
