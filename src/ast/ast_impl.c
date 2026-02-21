

#include "ast_impl.h"
#include <assert.h>
#include "libs/utilities/utilities.h"


static void iterate_data_stmts(const struct cst_node *cst, struct ast_file *ast) {
    assert(cst->kind == CST_DATA_STMTS);
    
    const struct cst_node *stmt = NULL;
    struct ast_data_stmt ast_stmt;
    for (size_t i = 0; i < cst->children.length; i++) {
        stmt = vec_get_ptr(&cst->children, i);
        build_ast_data_stmt(stmt, &ast_stmt);
        vec_push_u(&ast->data_stmts, &ast_stmt);
    }
}

static void iterate_exec_stmts(const struct cst_node *cst, struct ast_file *ast) {
    assert(cst->kind == CST_EXEC_STMTS);

    const struct cst_node *stmt = NULL;
    struct ast_exec_stmt ast_stmt;
    for (size_t i = 0; i < cst->children.length; i++) {
        stmt = vec_get_ptr(&cst->children, i);
        build_ast_exec_stmt(stmt, &ast_stmt);
        vec_push_u(&ast->exec_stmts, &ast_stmt);
    }

}

static void iterate_sections(const struct cst_node *cst, struct ast_file *ast) {
    assert(cst->kind == CST_SECTIONS);

    const struct cst_node *section = NULL;
    const struct cst_node *sub_section = NULL;
    const struct cst_node *stmts = NULL;
    for (size_t i = 0; i < cst->children.length; i++) {
        section = vec_get_ptr(&cst->children, i);
        sub_section = vec_get_ptr(&section->children, 0);
        stmts = vec_get_ptr(&sub_section->children, 1);
        if (sub_section->kind == CST_DATA_SECTION) {
            iterate_data_stmts(stmts, ast);
        } else {
            iterate_exec_stmts(stmts, ast);
        }
    }
}

void build_ast_file(const struct cst_node *cst, struct ast_file *ast) {
    assert(cst->kind == CST_FILE);

    vec_init_u(&ast->data_stmts, 10, sizeof(struct ast_data_stmt));
    vec_init_u(&ast->exec_stmts, 10, sizeof(struct ast_exec_stmt));

    const struct cst_node *sections = vec_get_ptr(&cst->children, 0);
    iterate_sections(sections, ast);

}

void build_ast_exec_stmt(const struct cst_node *cst, struct ast_exec_stmt *ast) {
    assert(cst->kind == CST_EXEC_STMT);

    const struct cst_node *stmt = vec_get_ptr(&cst->children, 0);
    switch (stmt->kind) {
        case CST_INSTRUCTION_STMT:
        ast->kind = E_STMT_INSTR;
        build_ast_instr_stmt(stmt, &ast->instr_stmt);
        break;

        case CST_LABEL_STMT:
        ast->kind = E_STMT_LABEL;
        build_ast_label_stmt(stmt, &ast->label_stmt);
        break;

        case CST_LOC_LABEL_STMT:
        ast->kind = E_STMT_L_LABEL;
        build_ast_l_label_stmt(stmt, &ast->l_label_stmt);
        break;

        case CST_MACRO_STMT:
        ast->kind = E_STMT_MACRO;
        build_ast_macro_stmt(stmt, &ast->macro_stmt);
        break;

        default:
        break;
    }

}



static void iterate_instr_args(const struct cst_node *cst, struct ast_instr_stmt *ast) {
    assert(cst->kind == CST_ARGS);

    const struct cst_node *arg_node = NULL;
    struct ast_arg ast_arg;
    for (size_t i = 0; i < cst->children.length; i += 2) {
        arg_node = vec_get_ptr(&cst->children, i);
        build_ast_arg(arg_node, &ast_arg);
        vec_push_u(&ast->args, &ast_arg);
    }
}

static void iterate_macro_args(const struct cst_node *cst, struct ast_macro_stmt *ast) {
    assert(cst->kind == CST_ARGS);

    const struct cst_node *arg_node = NULL;
    struct ast_arg ast_arg;
    for (size_t i = 0; i < cst->children.length; i += 2) {
        arg_node = vec_get_ptr(&cst->children, i);
        build_ast_arg(arg_node, &ast_arg);
        vec_push_u(&ast->args, &ast_arg);
    }
}

void build_ast_instr_stmt(const struct cst_node *cst, struct ast_instr_stmt *ast) {
    assert(cst->kind == CST_INSTRUCTION_STMT);

    vec_init_u(&ast->args, 10, sizeof(struct ast_arg));

    const struct cst_node *mnemonic_node = vec_get_ptr(&cst->children, 0);
    ast->mnemonic = strdups(mnemonic_node->terminal.lexeme);


    size_t index = 1;

    const struct cst_node *cond_node = vec_get_ptr(&cst->children, 1);
    if (cond_node->kind == CST_CONDITION_CODE) {
        const struct cst_node *sub_cond_node = vec_get_ptr(&cond_node->children, 1);
        enum ast_condition_code cond = get_ast_cond_code(sub_cond_node->terminal.lexeme);
        ast->cond = cond;
        index += 1;
    } else {
        ast->cond = CC_AL;
    }

    const struct cst_node *args_node = vec_get_ptr(&cst->children, index);
    iterate_instr_args(args_node, ast);

}

void build_ast_macro_stmt(const struct cst_node *cst, struct ast_macro_stmt *ast) {
    assert(cst->kind == CST_MACRO_STMT);

    vec_init_u(&ast->args, 10, sizeof(struct ast_arg));

    const struct cst_node *mnemonic_node = vec_get_ptr(&cst->children, 0);
    ast->mnemonic = strdups(mnemonic_node->terminal.lexeme);


    size_t index = 1;

    const struct cst_node *cond_node = vec_get_ptr(&cst->children, 1);
    if (cond_node->kind == CST_CONDITION_CODE) {
        const struct cst_node *sub_cond_node = vec_get_ptr(&cond_node->children, 1);
        enum ast_condition_code cond = get_ast_cond_code(sub_cond_node->terminal.lexeme);
        ast->cond = cond;
        index += 1;
    } else {
        ast->cond = CC_AL;
    }

    const struct cst_node *args_node = vec_get_ptr(&cst->children, index);
    iterate_macro_args(args_node, ast);
}

void build_ast_label_stmt(const struct cst_node *cst, struct ast_label_stmt *ast) {
    assert(cst->kind == CST_LABEL_STMT);

    const struct cst_node *label_node = vec_get_ptr(&cst->children, 0);
    ast->label = strdups(label_node->terminal.lexeme);
}

void build_ast_l_label_stmt(const struct cst_node *cst, struct ast_l_label_stmt *ast) {
    assert(cst->kind == CST_LOC_LABEL_STMT);

    const struct cst_node *label_node = vec_get_ptr(&cst->children, 1);
    ast->label = strdups(label_node->terminal.lexeme);
}

void build_ast_arg(const struct cst_node *cst, struct ast_arg *ast) {
    assert(cst->kind == CST_ARG);

    const struct cst_node *sub_arg = vec_get_ptr(&cst->children, 0);
    if (sub_arg->kind == CST_TERMINAL) {
        switch (sub_arg->terminal.kind) {
            case TOKEN_REG:
            ast->kind = ARG_REG;
            ast->reg = get_ast_reg(sub_arg->terminal.lexeme);
            break;

            case TOKEN_SYS_REG:
            ast->kind = ARG_SYS_REG;
            ast->sys_reg = get_ast_sys_reg(sub_arg->terminal.lexeme);
            break;

            case TOKEN_PORT:
            ast->kind = ARG_PORT;
            ast->port = get_ast_port(sub_arg->terminal.lexeme);
            break;

            case TOKEN_ADDR_REG:
            ast->kind = ARG_ADDR_REG;
            ast->addr_reg = get_ast_addr_reg(sub_arg->terminal.lexeme);
            break;

            default:
            break;
        }
    } else {
        const struct cst_node *sub_sub_arg = NULL;
        switch (sub_arg->kind) {
            case CST_IMMEDIATE:
            sub_sub_arg = vec_get_ptr(&sub_arg->children, 1);
            ast->kind = ARG_IMM;
            ast->imm = strdups(sub_sub_arg->terminal.lexeme);
            break;

            case CST_LABEL:
            sub_sub_arg = vec_get_ptr(&sub_arg->children, 0);
            ast->kind = ARG_LABEL;
            ast->label = strdups(sub_sub_arg->terminal.lexeme);
            break;

            case CST_LOC_LABEL:
            ast->kind = ARG_L_LABEL;
            build_ast_l_label(sub_arg, &ast->l_label);
            break;

            default:
            break;
        }
    }
}

void build_ast_l_label(const struct cst_node *cst, struct ast_l_label *ast) {
    assert(cst->kind == CST_LOC_LABEL);

    const struct cst_node *dir_node = vec_get_ptr(&cst->children, 0);
    const struct cst_node *sub_dir_node = vec_get_ptr(&dir_node->children, 0);
    if (strcmp(sub_dir_node->terminal.lexeme, ".f") == 0) {
        ast->back = false;
    } else {
        ast->back = true;
    }

    int index = 1;

    const struct cst_node *dist_node = vec_get_ptr(&cst->children, 1);
    if (dist_node->kind == CST_LOC_LABEL_DIST) {
        const struct cst_node *sub_dist_node = vec_get_ptr(&dist_node->children, 1);
        ast->dist = strdups(sub_dist_node->terminal.lexeme);
        index += 1;
    }

    const struct cst_node *label_node = vec_get_ptr(&cst->children, index);
    ast->label = strdups(label_node->terminal.lexeme);
}

void build_ast_data_stmt(const struct cst_node *cst, struct ast_data_stmt *ast) {
    assert(cst->kind == CST_DATA_STMT);
}

void build_ast_byte_stmt(const struct cst_node *cst, struct ast_byte_stmt *ast) {
    assert(cst->kind == CST_BYTE_STMT);
}

void build_ast_bytes_stmt(const struct cst_node *cst, struct ast_bytes_stmt *ast) {
    assert(cst->kind == CST_BYTES_STMT);
}

void build_ast_init(const struct cst_node *cst, struct ast_init *ast) {
    assert(cst->kind == CST_INITIALIZER);
}



enum ast_reg get_ast_reg(const char *str) {
    int reg;
    sscanf(str, "r%d", &reg);
    return REG_0 + reg;
}

enum ast_addr_reg get_ast_addr_reg(const char *str) {
    int reg;
    sscanf(str, "r%d", &reg);
    return REG_0_A + reg;
}

enum ast_sys_reg get_ast_sys_reg(const char *str) {
    if (strcmp(str, "pc_b0") == 0) {
        return PC_B0;
    } else if (strcmp(str, "pc_b1") == 0) {
        return PC_B1;
    } else if (strcmp(str, "psr") == 0) {
        return PSR;
    } else if (strcmp(str, "intr") == 0) {
        return INTR;
    } else if (strcmp(str, "pdbr_b0") == 0) {
        return PDBR_B0;
    } else if (strcmp(str, "pdbr_b1") == 0) {
        return PDBR_B1;
    } 
    return -1;
}

enum ast_port get_ast_port(const char *str) {
    int port;
    sscanf(str, "p%d", &port);
    return PORT_0 + port;
}

enum ast_condition_code get_ast_cond_code(const char *str) {
    if (strcmp(str, "al") == 0) {
        return CC_AL;
    }

    /* eq = zs */
    else if (strcmp(str, "eq") == 0 || strcmp(str, "zs") == 0) {
        return CC_EQ;
    }

    else if (strcmp(str, "mi") == 0) {
        return CC_MI;
    }

    else if (strcmp(str, "vs") == 0) {
        return CC_VS;
    }

    /* su = cc */
    else if (strcmp(str, "su") == 0 || strcmp(str, "cc") == 0) {
        return CC_SU;
    }

    else if (strcmp(str, "gu") == 0) {
        return CC_GU;
    }

    else if (strcmp(str, "ss") == 0) {
        return CC_SS;
    }

    else if (strcmp(str, "gs") == 0) {
        return CC_GS;
    }

    /* ne = zc */
    else if (strcmp(str, "ne") == 0 || strcmp(str, "zc") == 0) {
        return CC_NE;
    }

    else if (strcmp(str, "pl") == 0) {
        return CC_PL;
    }

    else if (strcmp(str, "vc") == 0) {
        return CC_VC;
    }

    /* geu = cs */
    else if (strcmp(str, "geu") == 0 || strcmp(str, "cs") == 0) {
        return CC_GEU;
    }

    else if (strcmp(str, "seu") == 0) {
        return CC_SEU;
    }

    else if (strcmp(str, "ges") == 0) {
        return CC_GES;
    }

    else if (strcmp(str, "ses") == 0) {
        return CC_SES;
    }

    return -1;
}

