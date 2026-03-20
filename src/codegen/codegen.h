
#ifndef __CODEGEN_HEADER__
#define __CODEGEN_HEADER__


#include "shared/ast.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void generate_binary(struct ast_file *file, uint16_t start, FILE *out);



#endif
