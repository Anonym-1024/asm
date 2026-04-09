
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
    uint32_t code_len;
    uint32_t data_len;
};



struct compiled_instruction {
    uint8_t bytes[4];

    char *label;
};


struct linker_context {
    struct object_file *objs;
    int objs_n;
    FILE *out;

    uint32_t total_code_len;
    uint32_t total_data_len;



    struct hashmap global_symbol_table;

    char err_msg[ERR_MSG_LEN];
    const char *current_file;

};

static enum linker_result read_str(FILE *f, char **out) {
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

static enum linker_result read_int32(FILE *f, uint32_t *n) {
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

static enum linker_result read_instr(FILE *f, struct compiled_instruction *instr) {
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

static enum linker_result open_obj_files(const char **files, int files_n, struct object_file **objs, struct compiler_error *err) {

    *objs = malloc(sizeof(struct object_file) * files_n);
    if (*objs == NULL) {
        return LINK_ERR;
    }
    struct object_file *_objs = *objs;
    for (int i = 0; i < files_n; i++) {
        _objs[i].file = NULL;
        _objs[i].filename = files[i];
    }


    for (int i = 0; i < files_n; i++) {
        err->file = files[i];
        _objs[i].file = fopen(files[i], "rb");
        if (_objs[i].file == NULL) {
            snprintf(err->msg, ERR_MSG_LEN, "Could not open object file.");
            goto _error;
        }
        try_else(read_int32(_objs[i].file, &_objs[i].symbol_table_len), LINK_OK, goto _error);
        try_else(read_int32(_objs[i].file, &_objs[i].code_len), LINK_OK, goto _error);
        try_else(read_int32(_objs[i].file, &_objs[i].data_len), LINK_OK, goto _error);
    }

    return LINK_OK;

_error:

    close_obj_files(_objs, files_n);
    return LINK_ERR;
}


static enum linker_result build_symbol_table(struct linker_context *ctx) {

    try_else(hashmap_init(&ctx->global_symbol_table, 256), HMAP_OK, return LINK_ERR);






    uint32_t global_code_offset = 4;
    uint32_t global_data_offset = 0;
    for (int i = 0; i < ctx->objs_n; i++) {
        struct object_file *obj = &ctx->objs[i];
        ctx->current_file = obj->filename;
        for (uint32_t j = 0; j < obj->symbol_table_len; j++) {
            char *key;
            try_else(read_str(obj->file, &key), LINK_OK, goto _error);
            if (hashmap_find(&ctx->global_symbol_table, key) == HMAP_OK) {
                snprintf(ctx->err_msg, ERR_MSG_LEN, "Found label '%.20s' redefinition.", key);

                free(key);
                goto _error;
            }

            uint32_t offset;
            try_else(read_int32(obj->file, &offset), LINK_OK, goto _error);
            if (offset < obj->code_len) {
                offset += global_code_offset;
            } else {
                offset = (offset - obj->code_len) + ctx->total_code_len + global_data_offset;
            }


            try_else(hashmap_add(&ctx->global_symbol_table, key, offset), HMAP_OK, goto _error);
            free(key);
        }
        global_data_offset += obj->data_len;
        global_code_offset += obj->code_len;

    }

    return LINK_OK;

_error:
    hashmap_deinit(&ctx->global_symbol_table);
    return LINK_ERR;
}


static enum linker_result resolve_label(struct linker_context *ctx, struct compiled_instruction *instr, uint32_t global_code_offset, uint32_t global_data_offset, uint32_t code_len) {

    uint32_t offset;
    if (strcmp(instr->label, "") == 0) {

    } else if (strcmp(instr->label, "~") == 0) {
        offset = 0;
        offset |= instr->bytes[2];
        offset |= instr->bytes[3] << 8;


        if (offset < code_len) {

            offset += global_code_offset;
            instr->bytes[2] = offset;
            instr->bytes[3] = offset >> 8;
        } else {
            offset = (offset - code_len) + ctx->total_code_len + global_data_offset;
            instr->bytes[2] = offset;
            instr->bytes[3] = offset >> 8;
        }


    } else {
        if (hashmap_get(&ctx->global_symbol_table, instr->label, &offset) != HMAP_OK) {
            snprintf(ctx->err_msg, ERR_MSG_LEN, "Undefined symbol: %s", instr->label);
            return LINK_ERR;
        }

        instr->bytes[2] = offset;
        instr->bytes[3] = offset >> 8;

    }
    return LINK_OK;
}

static enum linker_result write_code_sections(struct linker_context *ctx) {

    uint32_t global_code_offset = 4;
    uint32_t global_data_offset = 0;

    struct compiled_instruction instr;
    instr.label = NULL;

    for (int i = 0; i < ctx->objs_n; i++) {
        struct object_file *obj = &ctx->objs[i];
        ctx->current_file = obj->filename;

        for (uint32_t j = 0; j < obj->code_len; j+=4) {
            try_else(read_instr(obj->file, &instr), LINK_OK, goto _error)
            try_else(resolve_label(ctx, &instr, global_code_offset, global_data_offset, obj->code_len), LINK_OK, goto _error);
            try_else(fwrite(&instr.bytes, sizeof(uint8_t), 4, ctx->out), 4, goto _error);
            free(instr.label);
            instr.label = NULL;
        }
        global_data_offset += obj->data_len;
        global_code_offset += obj->code_len;

    }

    return LINK_OK;

_error:
    free(instr.label);
    return LINK_ERR;
}

static enum linker_result write_data_sections(struct linker_context *ctx) {

    for (int i = 0; i < ctx->objs_n; i++) {
        struct object_file *obj = &ctx->objs[i];
        ctx->current_file = obj->filename;
        uint8_t byte;
        while (fread(&byte, sizeof(uint8_t), 1, obj->file) == 1) {

            try_else(fwrite(&byte, sizeof(uint8_t), 1, ctx->out), 1, return LINK_ERR);
        }
    }

    return LINK_OK;
}




static enum linker_result calculate_total_len(struct linker_context *ctx) {
    ctx->total_code_len = 4;
    ctx->total_data_len = 0;
    for (int i = 0; i < ctx->objs_n; i++) {
        struct object_file *obj = &ctx->objs[i];
        ctx->total_code_len += obj->code_len;
        ctx->total_data_len += obj->data_len;
    }
    if (ctx->total_data_len + ctx->total_code_len > UINT32_MAX) {
        snprintf(ctx->err_msg, ERR_MSG_LEN, "Binary size exceeds maximum addressable space.");
        return LINK_ERR;
    }
    return LINK_OK;
}



enum linker_result link_object_files(const char **files, int files_n, const char *out, struct compiler_error *err) {

    struct linker_context ctx = {
        .current_file = "",
        .objs_n = files_n,
        .total_code_len = 0,
        .total_data_len = 0,

    };

    strcpy(ctx.err_msg, "Unknown error.");
    err->file = NULL;

    bool _table = false;
    bool _objs = false;
    bool _fout = false;

    try_else(open_obj_files(files, files_n, &ctx.objs, err), LINK_OK, goto _error);
    _objs = true;


    try_else(calculate_total_len(&ctx), LINK_OK, goto _error);
    try_else(build_symbol_table(&ctx), LINK_OK, goto _error);
    _table = true;


    uint32_t start;
    if (hashmap_get(&ctx.global_symbol_table, ".start", &start) == HMAP_NO_ENTRY) {
        snprintf(ctx.err_msg, ERR_MSG_LEN, "Symbol '.start' has not been defined.");
        goto _error;
    }




    if (out == NULL) {
        out = "a.bin";
    }
    ctx.out = fopen(out, "wb");
    if (ctx.out == NULL) {
        goto _error;
    }


    try_else(fwrite(&(uint8_t[]){0x05, 0xb0, start, start>>8}, sizeof(uint8_t), 4, ctx.out), 4, return LINK_ERR);

    try_else(write_code_sections(&ctx), LINK_OK, goto _error);
    try_else(write_data_sections(&ctx), LINK_OK, goto _error);


    close_obj_files(ctx.objs, files_n);
    fclose(ctx.out);
    hashmap_deinit(&ctx.global_symbol_table);
    return LINK_OK;

_error:

    if (_objs) {
        close_obj_files(ctx.objs, files_n);
    }

    if (_table) {
        hashmap_deinit(&ctx.global_symbol_table);
    }

    if (_fout) {
        fclose(ctx.out);
    }

    err->col = 0;
    err->line = 0;
    err->kind = CERROR_LINKER;
    err->file = ctx.current_file;
    strncpy(err->msg, ctx.err_msg, ERR_MSG_LEN);
    return LINK_ERR;
}
