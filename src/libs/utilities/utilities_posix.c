
#include "utilities.h"

#if defined(__unix__) || defined(__APPLE__)

#include <sys/stat.h>
#include <unistd.h>



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


#endif