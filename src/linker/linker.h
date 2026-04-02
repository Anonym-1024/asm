#ifndef __LINKER_HEADER__
#define __LINKER_HEADER__








#include "error/compiler_error.h"
#include <stdio.h>
enum linker_result {
    LINK_OK,
    LINK_ERR
};

enum linker_result link_object_files(const char **files, int files_n, const char *out, struct compiler_error *err);

#endif
