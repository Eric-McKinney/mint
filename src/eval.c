#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "parser.h"

#define MAX_PARAMS 50 /* arbitrary upper limit on how many params a function can have */

Env_t *init_env() {
    return calloc(1, sizeof(Env_t));
}

static ExprTree *copy_expr_tree(ExprTree *tree) {
    ExprTree *copy, *left_copy, *right_copy;

    if (tree == NULL) {
        return NULL;
    }

    left_copy = copy_expr_tree(tree->left);
    right_copy = copy_expr_tree(tree->right);

    copy = malloc(sizeof(ExprTree));
    copy->expr = tree->expr;

    switch (tree->expr) {
        case ID:
            copy->value.id = malloc(strlen(tree->value.id) + 1);
            strcpy(copy->value.id, tree->value.id);
            break;
        case Int:
        case Float:
        case Fun:
        case Binop:
            copy->value = tree->value;
            break;
        default:
            break;
    }

    copy->left = left_copy;
    copy->right = right_copy;

    return copy;
}

static void extend_env(Env_t *env, const char *id, ExprTree *data) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = copy_expr_tree(data);
    new_data->next = env->next;

    env->next = new_data;
}

static void extend_env_tmp(Env_t *env, const char *id) {
    ExprTree *dummy_data = calloc(1, sizeof(ExprTree));

    dummy_data->expr = ID;
    dummy_data->value.id = malloc(strlen(id) + 1);
    strcpy(dummy_data->value.id, id);

    extend_env(env, id, dummy_data);

    free_expr_tree(dummy_data);
}

static Env_t *env_find(Env_t *env, const char *id, Env_t **prev) {
    Env_t *p = env;

    env = env->next;
    while (env != NULL) {
        if (strcmp(env->id, id) == 0) {
            if (prev != NULL) {
                *prev = p;
            }

            return env;
        }

        p = env;
        env = env->next;
    }

    return NULL;
}

static ExprTree *lookup(Env_t *env, const char *id) {
    Env_t *e = env_find(env, id, NULL);

    if (e != NULL) {
        return copy_expr_tree(e->data);
    } else {
        fprintf(stderr, "Unbound identifier %s\n", id);
        exit(EXIT_FAILURE);
    }
}

static void free_env_node(Env_t *env) {
    free(env->id);
    free_expr_tree(env->data);
    free(env);
}

void free_env(Env_t *env) {
    if (env == NULL) {
        return;
    }

    free_env(env->next);
    free_env_node(env);
}

static void shrink_env(Env_t *env, const char *id, int qty) {
    Env_t *e, *prev, *last;
    int i;

    e = env_find(env, id, &prev);

    if (e == NULL) {
        fprintf(stderr, "Failed to shrink environment: cannot find identifier %s\n", id);
        exit(EXIT_FAILURE);
    }

    last = e;
    for (i = 1; i < qty; i++) {
        last = last->next;
    }

    prev->next = last->next;
    last->next = NULL;
    free_env(e);
}

static void update_env(Env_t *env, const char *id, ExprTree *new_data) {
    Env_t *env_entry = env_find(env, id, NULL);

    free_expr_tree(env_entry->data);
    env_entry->data = copy_expr_tree(new_data);
}

static char *env_to_str_aux(Env_t *env) {
    char *str, *data_str, *env_str;
    int env_str_len, total_len;

    if (env == NULL) {
        return calloc(1, 1); /* effectively returns "" */
    }

    env_str = env_to_str_aux(env->next);
    data_str = expr_tree_to_str(env->data);
    
    env_str_len = strlen(env_str);
    total_len = strlen(env->id) + strlen(data_str) + env_str_len;
    
    /* the + 8 at the end is for null char, parens, colon, spaces, and potentially a comma */
    str = malloc(total_len + 8);
    sprintf(str, "(%s : %s)", env->id, data_str);

    if (env_str_len != 0) {
        sprintf(str + strlen(str), ", %s", env_str);
    }
    
    free(data_str);
    free(env_str);

    return str;
}

char *env_to_str(Env_t *env) {
    char *str, *contents;

    env = env->next;
    contents = env_to_str_aux(env);

    str = malloc(strlen(contents) + 3);

    sprintf(str, "[%s]", contents);
    free(contents);

    return str;
}

static void eval_fun(ExprTree **t, Env_t *env);
static void eval_binop(ExprTree **t, Env_t *env);
static void eval_assign(ExprTree *tree, Env_t *env);
static void eval_application(ExprTree **t, Env_t *env);
static void eval_arguments(ExprTree **t, Env_t *env);
static int push_params(ExprTree *params, Env_t *env);
static int bind_args(ExprTree *args, ExprTree *params, Env_t *env);
static void pop_params(ExprTree *params, int num_params, Env_t *env);

ExprTree *eval(ExprTree **tree, Env_t *env) {
    switch ((*tree)->expr) {
        case Int:
        case Float:
            return *tree;
        case ID: {
            ExprTree *value = lookup(env, (*tree)->value.id);
            free_expr_tree(*tree);
            *tree = value;
            break;
        }
        case Fun:
            eval_fun(tree, env);
            break;
        case Binop:
            eval_binop(tree, env);
            break;
        case Assign:
            eval_assign(*tree, env);
            break;
        case Application:
            eval_application(tree, env);
            break;
        case Argument:
            fprintf(stderr, "Arugment expression outside of function application\n");
            exit(EXIT_FAILURE);
        case Parameter:
            fprintf(stderr, "Parameter expression outside of function definition\n");
            exit(EXIT_FAILURE);
        default: {
            char *expr_str = expr_tree_to_str(*tree);
            fprintf(stderr, "Failed to evaluate unrecognized expression: %s\n", expr_str);
            free(expr_str);
            exit(EXIT_FAILURE);
        }
    }
    
    return *tree;
}

static void validate_params(ExprTree *params, const char *fun_id) {
    char *p[MAX_PARAMS];
    int i = 0, j, dupes = 0;

    while (params != NULL) {
        p[i] = params->left->value.id;

        for (j = 0; j < i; j++) {
            if (strcmp(p[j], p[i]) == 0) {
                fprintf(stderr, "Duplicate parameter \"%s\" in %s\n", p[i], fun_id);
                dupes++;
                break;
            }
        }

        i++;
        params = params->right;
    }

    if (dupes) {
        exit(EXIT_FAILURE);
    }
}

static void eval_fun(ExprTree **t, Env_t *env) {
    ExprTree *tree = *t;
    int num_params;

    num_params = push_params(tree->left, env);
    eval(&(tree->right), env);
    validate_params(tree->left, tree->value.id);
    extend_env(env, tree->value.id, tree);
    pop_params(tree->left, num_params, env);
}

static void eval_binop(ExprTree **t, Env_t *env) {
    ExprTree *tree = *t;
    ExprTree *v1, *v2, *v;
    int can_simplify, v1_is_int, v1_is_float, v2_is_int, v2_is_float;

    v1 = eval(&(tree->left), env);
    v2 = eval(&(tree->right), env);

    v1_is_int = v1->expr == Int;
    v1_is_float = v1->expr == Float;
    v2_is_int = v2->expr == Int;
    v2_is_float = v2->expr == Float;
    can_simplify = (v1_is_int || v1_is_float) && (v2_is_int || v2_is_float);

    if (!can_simplify) {
        return;
    }

    v = malloc(sizeof(ExprTree));
    v->expr = (v1_is_float || v2_is_float) ? Float : Int;
    v->left = NULL;
    v->right = NULL;

    switch (tree->value.binop) {
        case Add:
            if (v->expr == Int) {
                v->value.i = v1->value.i + v2->value.i;
            } else {
                if (v1_is_float && v2_is_int) {
                    v->value.d = v1->value.d + v2->value.i;
                } else if (v1_is_int && v2_is_float) {
                    v->value.d = v1->value.i + v2->value.d;
                } else {
                    v->value.d = v1->value.d + v2->value.d;
                }
            }

            break;
        case Sub:
            if (v->expr == Int) {
                v->value.i = v1->value.i - v2->value.i;
            } else {
                if (v1_is_float && v2_is_int) {
                    v->value.d = v1->value.d - v2->value.i;
                } else if (v1_is_int && v2_is_float) {
                    v->value.d = v1->value.i - v2->value.d;
                } else {
                    v->value.d = v1->value.d - v2->value.d;
                }
            }

            break;
        case Mult:
            if (v->expr == Int) {
                v->value.i = v1->value.i * v2->value.i;
            } else {
                if (v1_is_float && v2_is_int) {
                    v->value.d = v1->value.d * v2->value.i;
                } else if (v1_is_int && v2_is_float) {
                    v->value.d = v1->value.i * v2->value.d;
                } else {
                    v->value.d = v1->value.d * v2->value.d;
                }
            }

            break;
        case Div:
            if (v->expr == Int) {
                if (v1->value.i % v2->value.i == 0) {
                    v->value.i = v1->value.i / v2->value.i;
                } else {
                    v->expr = Float;
                    v->value.d = (double) v1->value.i / (double) v2->value.i;
                }
            } else {
                if (v1_is_float && v2_is_int) {
                    v->value.d = v1->value.d / v2->value.i;
                } else if (v1_is_int && v2_is_float) {
                    v->value.d = v1->value.i / v2->value.d;
                } else {
                    v->value.d = v1->value.d / v2->value.d;
                }
            }
            
            break;
        default: {
            char *expr_str = expr_tree_to_str(tree);
            fprintf(stderr, "Failed to evaluate unrecognized binary operator expression: %s\n", expr_str);
            free(expr_str);
            exit(EXIT_FAILURE);
        }
    }

    free_expr_tree(tree);
    *t = v;
}

static void eval_assign(ExprTree *tree, Env_t *env) {
    ExprTree *value = eval(&(tree->right), env);
    
    extend_env(env, tree->left->value.id, value);
}

static void eval_application(ExprTree **t, Env_t *env) {
    ExprTree *tree = *t;
    ExprTree *fun = lookup(env, tree->left->value.id);
    ExprTree *fun_body = fun->right;
    ExprTree *params = fun->left;
    ExprTree *args = tree->right;
    ExprTree *ret_val;
    int num_params, num_args_bound;
    
    eval_arguments(&(args), env);
    num_params = push_params(params, env);
    num_args_bound = bind_args(args, params, env);

    if (num_params != num_args_bound) {
        fprintf(stderr,
                "In application of %s: received %d arguments, expected %d\n",
                fun->value.id,
                num_args_bound,
                num_params);
        exit(EXIT_FAILURE);
    }

    ret_val = eval(&fun_body, env);
    pop_params(params, num_params, env);

    tree->right = NULL;
    free_expr_tree(tree);
    *t = ret_val;
}

static void eval_arguments(ExprTree **t, Env_t *env) {
    ExprTree *arg = *t;

    while (arg != NULL) {
        eval(&(arg->left), env);
        arg = arg->right;
    }
}

static int push_params(ExprTree *params, Env_t *env) {
    if (params == NULL) {
        return 0;
    }

    extend_env_tmp(env, params->left->value.id);

    return 1 + push_params(params->right, env);
}

static int bind_args(ExprTree *args, ExprTree *params, Env_t *env) {
    int args_binded;

    if (args == NULL || params == NULL) {
        return 0;
    }

    args_binded = bind_args(args->right, params->right, env);
    update_env(env, params->left->value.id, args->left);

    return 1 + args_binded;
}

static void pop_params(ExprTree *params, int num_params, Env_t *env) {
    char *top_param; /* first param in env (top of stack) */

    while (params->right != NULL) {
        params = params->right;
    }

    top_param = params->left->value.id;
    shrink_env(env, top_param, num_params);
}
