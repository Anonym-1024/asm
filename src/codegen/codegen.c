 
#include "codegen.h"




struct codegen_context {
    uint32_t exec_offset;
    uint32_t data_offset;
    FILE *obj;
};





enum codegen_result generate_instruction_stmt(struct ast_instruction_stmt *stmt, struct codegen_context *ctx) {

}

enum codegen_result generate_exec_stmt(struct ast_exec_stmt *stmt, struct codegen_context *ctx) {

}

enum codegen_result generate_exec_stmts(struct ast_exec_section *sec, struct codegen_context *ctx) {

}

enum codegen_result generate_exec_sections(struct ast_file *file, struct codegen_context *ctx) {

}




enum codegen_result generate_label_stmt(struct ast_label_stmt *stmt, struct codegen_context *ctx) {

}

enum codegen_result generate_bytes_stmt(struct ast_bytes_stmt *stmt, struct codegen_context *ctx) {
    
}

enum codegen_result generate_byte_stmt(struct ast_byte_stmt *stmt, struct codegen_context *ctx) {

}

enum codegen_result generate_data_stmt(struct ast_data_stmt *stmt, struct codegen_context *ctx) {

}

enum codegen_result generate_data_stmts(struct ast_data_section *sec, struct codegen_context *ctx) {

}

enum codegen_result generate_data_sections(struct ast_file *file, struct codegen_context *ctx) {

}



enum codegen_result generate_compiled_object(struct ast_file *file, FILE *obj, struct compiler_error *err) {


    struct codegen_context ctx = {
        .data_offset = 0,
        .exec_offset = 0,
        .obj = obj
    };

    



    // scan data files
    // scan exec files
    


    return CODEGEN_OK;

_error:
    return CODEGEN_ERR;
}

