#ifndef __COMPILATION_HEADER__
#define __COMPILATION_HEADER__


#include "error/compiler_error.h"
#include "shared/source_file.h"

enum compilation_result {
    COMP_OK,
    COMP_ERR
};



enum compilation_result compile_source_file(const char *in, const char *out, struct compiler_error *err);


#endif
