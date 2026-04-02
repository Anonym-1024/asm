
#include "linker.h"
#include "error/compiler_error.h"
#include "libs/error_handling.h"
#include "libs/hashmap/hashmap.h"
#include "libs/vector/vector.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



struct object_file {
    const char *filename;
    FILE *file;
    uint32_t symbol_table_len;
    uint32_t exec_len;
    uint32_t data_len;
};



struct compiled_instruction {
    uint8_t bytes[4];
    
    char *label;
};


enum linker_result read_str(FILE *f, char **out) {
    struct vector buffer;
    try_else(vec_init(&buffer, 10, sizeof(char)), VEC_OK, return LINK_ERR);
    
    char c;
    while (fread(&c, sizeof(char), 1, f) == 1 && c != '\0') {
        try_else(vec_push(&buffer, &c), VEC_OK, goto _error);
    }
    try_else(vec_push(&buffer, &(char){0}), VEC_OK, goto _error);

    *out = buffer.ptr;
    return LINK_OK;

_error:
    vec_deinit(&buffer, NULL);
    return LINK_ERR;
}

enum linker_result read_int32(FILE *f, uint32_t *n) {
    uint8_t bytes[4];

    
    if (fread(bytes, 1, 4, f) != 4) {
        return LINK_ERR;
    }

    
    *n = (int32_t)(
        ((uint32_t)bytes[0]) |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) |
        ((uint32_t)bytes[3] << 24)
    );

    return LINK_OK;
}

enum linker_result read_instr(FILE *f, struct compiled_instruction *instr) {
    uint8_t bytes[4];

    
    if (fread(bytes, 1, 4, f) != 4) {
        return LINK_ERR;
    }

    
    instr->bytes[0] = bytes[0];
    instr->bytes[1] = bytes[1];
    instr->bytes[2] = bytes[2];
    instr->bytes[3] = bytes[3];

    try_else(read_str(f, &instr->label), LINK_OK, return LINK_ERR);


    return LINK_OK;
}

static void close_obj_files(struct object_file *objs, int obj_n) {
    if (objs == NULL) {
        return;
    }
    for (int i = 0; i < obj_n; i++) {
        fclose(objs[i].file);
    }
}

enum linker_result open_obj_files(const char **files, int files_n, struct object_file **objs, struct compiler_error *err) {

    *objs = malloc(sizeof(struct object_file) * files_n);
    if (objs == NULL) {
        return LINK_ERR;
    }
    for (int i = 0; i < files_n; i++) {
        objs[i]->file = NULL;
    }

    struct object_file *_objs = *objs;
    for (int i = 0; i < files_n; i++) {
        err->file = files[i];
        objs[i]->file = fopen(files[i], "rb");
        if (objs[i]->file == NULL) {
            goto _error;
        }
        try_else(read_int32(_objs[i].file, &_objs[i].symbol_table_len), LINK_OK, goto _error);
        try_else(read_int32(_objs[i].file, &_objs[i].exec_len), LINK_OK, goto _error);
        try_else(read_int32(_objs[i].file, &_objs[i].data_len), LINK_OK, goto _error);
    }

    return LINK_OK;

_error:

    close_obj_files(_objs, files_n);
    return LINK_ERR;
}


enum linker_result build_symbol_table(struct object_file *objs, int obj_n, struct hashmap *table, struct compiler_error *err) {

    try_else(hashmap_init(table, 256), HMAP_OK, return LINK_ERR);

    uint32_t total_exec_len = 0;
    uint32_t total_data_len = 0;

    for (int i = 0; i < obj_n; i++) {
        struct object_file *obj = &objs[i];
        total_exec_len += obj->exec_len;
        total_data_len += obj->data_len;
    }

    if (total_data_len + total_exec_len > UINT32_MAX) {
        snprintf(err->msg, ERR_MSG_LEN, "Executable size exceeds maximum addressable space.");
        goto _error;
    }

    
    uint32_t global_exec_offset = 0;
    uint32_t global_data_offset = 0;
    for (int i = 0; i < obj_n; i++) {
        struct object_file *obj = &objs[i];
        err->file = obj->filename;
        for (uint32_t j = 0; j < obj->symbol_table_len; j++) {
            char *key;
            try_else(read_str(obj->file, &key), LINK_OK, goto _error);
            uint32_t offset;
            try_else(read_int32(obj->file, &offset), LINK_OK, goto _error);
            if (offset < obj->exec_len) {
                offset += global_exec_offset;
            } else {
                offset = (offset - obj->exec_len + total_exec_len) + global_data_offset;
            }
            if (hashmap_find(table, key) == HMAP_OK) {
                snprintf(err->msg, ERR_MSG_LEN, "Found label '%.20s' redefinition.", key);
                
                free(key);
                goto _error;
            }

            try_else(hashmap_add(table, key, offset), HMAP_OK, goto _error);
            free(key);
        }
        global_data_offset += obj->data_len;
        global_exec_offset += obj->exec_len;
        
    }

    return LINK_OK;

_error:
    hashmap_deinit(table);
    return LINK_ERR;
}





enum linker_result write_exec_sections(struct object_file *objs, int obj_n, struct hashmap *table, FILE *fout, struct compiler_error *err) {
    struct compiled_instruction instr;
    instr.label = NULL;

    uint32_t global_exec_offset = 0;
    for (int i = 0; i < obj_n; i++) {
        struct object_file *obj = &objs[i];
        err->file = obj->filename;
        
        while (read_instr(obj->file, &instr) == LINK_OK) {
            if (*instr.label == '\0') {
                try_else(fwrite(&instr.bytes, sizeof(uint8_t), 4, fout), 4, goto _error);
                
            } else if (*instr.label == '~') {
                uint16_t new_offset = 0;
                new_offset |= (uint16_t)instr.bytes[2];
                new_offset |= (uint16_t)instr.bytes[3] << 8;
                new_offset += global_exec_offset;
                instr.bytes[3] = (new_offset >> 8);
                instr.bytes[2] = new_offset;
                try_else(fwrite(&instr.bytes, sizeof(uint8_t), 4, fout), 4, goto _error);
            } else {
                uint32_t new_offset;;
                if (hashmap_get(table, instr.label, &new_offset) != HMAP_OK) {
                    snprintf(err->msg, ERR_MSG_LEN, "Label '%.20s' has not been defined in any file.", instr.label);
                    goto _error;
                }
                instr.bytes[3] = (new_offset >> 8);
                instr.bytes[2] = new_offset;
                try_else(fwrite(&instr.bytes, sizeof(uint8_t), 4, fout), 4, goto _error);
            }
            free(instr.label);
            instr.label = NULL;
        }
        global_exec_offset += obj->exec_len;
        
    }

_error: 
    free(instr.label);
    return LINK_ERR;
}

enum linker_result write_data_sections(struct object_file *objs, int obj_n, FILE *fout, struct compiler_error *err) {
    for (int i = 0; i < obj_n; i++) {
        struct object_file *obj = &objs[i];
        err->file = obj->filename;
        uint8_t byte;
        while (fread(&byte, sizeof(uint8_t), 1, obj->file) == 1) {
            try_else(fwrite(&byte, sizeof(uint8_t), 1, fout), 1, return LINK_ERR);
        }
    }

    return LINK_OK;
}






enum linker_result link_object_files(const char **files, int files_n, const char *out, struct compiler_error *err) {

    strcpy(err->msg, "Unknown error.");
    err->file = NULL;

    struct object_file *objs;
    bool _table = false;
    bool _objs = false;
    bool _fout = false;

    try_else(open_obj_files(files, files_n, &objs, err), LINK_OK, goto _error);
    _objs = true;

    
    
    struct hashmap symbol_table;
    try_else(build_symbol_table(objs, files_n, &symbol_table, err), LINK_OK, goto _error);
    _table = true;

    FILE *fout = fopen(out, "wb");
    if (fout == NULL) {
        goto _error;
    }


    try_else(write_exec_sections(objs, files_n, &symbol_table, fout, err), LINK_OK, goto _error);
    try_else(write_data_sections(objs, files_n, fout, err), LINK_OK, goto _error);


    close_obj_files(objs, files_n);
    fclose(fout);
    return LINK_OK;

_error:

    if (_objs) {
        close_obj_files(objs, files_n);
    }

    if (_table) {
        hashmap_deinit(&symbol_table);
    }

    if (_fout) {
        fclose(fout);
    }

    err->col = 0;
    err->line = 0;
    err->kind = CERROR_LINKER;
    return LINK_ERR;
}
