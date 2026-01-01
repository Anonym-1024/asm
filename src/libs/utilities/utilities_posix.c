
#include "utilities.h"

#if defined(__unix__) || defined(__APPLE__)

#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>


off_t get_file_size(const char *path) {
    struct stat st;
    stat(path, &st);
    return st.st_size;
}


bool contains_str(const char **array, size_t n, const char *element) {
    for (int i = 0; i < n; i++) {
        if (strcmp(array[i], element) == 0) {
            return true;
        }
    }
    return false;
}

void *mallocs(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        FATAL_ERR;
    }
    return p;
}

void asprintfs(char **strp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (vasprintf(strp, fmt, args) == -1) {
        va_end(args);
        FATAL_ERR;
    }

    va_end(args);
}

#endif