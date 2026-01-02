

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


#define FATAL_ERR {printf("fatal error."); exit(1);}


off_t get_file_size(const char *path);

bool contains_str(const char **array, size_t n, const char *element);

void *mallocs(size_t size);

void asprintfs(char **restrict strp, const char *restrict fmt, ...);
