#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "parser.h"

Env_t *init_env() {
    return calloc(1, sizeof(Env_t));
}

static void extend_env(Env_t *env, const char *id, ExprTree *data) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = data;
    new_data->next = env->next;

    env->next = new_data;
}

static void extend_env_tmp(Env_t **env, const char *id) {
    extend_env(env, id, NULL);
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
        return e->data;
    } else {
        fprintf(stderr, "Unbound identifier %s\n", id);
        exit(EXIT_FAILURE);
    }
}

static void free_env_node(Env_t *env) {
    free(env->id);
    free(env);
}

void free_env(Env_t *env) {
    if (env == NULL) {
        return;
    }

    free_env(env->next);
    free_env_node(env);
}

static void shrink_env(Env_t **env, const char *id, int qty) {
    Env_t *e, *prev, *last;
    int i;

    e = env_find(*env, id, &prev);

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

static void update_env(Env_t *env, const char *id, ExprTree *data) {
    ExprTree **old_data = &lookup(env, id);

    *old_data = data;
}

static ExprTree *eval_fun(ExprTree *tree, Env_t *env);
static ExprTree *eval_binop(ExprTree *tree, Env_t *env);
static ExprTree *eval_assign(ExprTree *tree, Env_t *env);
static ExprTree *eval_application(ExprTree *tree, Env_t *env);
static ExprTree *push_params(ExprTree *tree, Env_t *env);
static ExprTree *eval_arguments(ExprTree *tree, Env_t *env);

ExprTree *eval(ExprTree *tree, Env_t *env) {
    switch (tree->expr) {
        case Int:
        case Float:
            return tree;
        case ID: 
            return lookup(env, tree->value.id);
        case Fun:
            return eval_fun(tree, env);
        case Binop:
            return eval_binop(tree, env);
        case Assign:
            return eval_assign(tree, env);
        case Application:
            return eval_application(tree, env);
        case Argument:
            fprintf(stderr, "Arugment expression outside of function application\n");
            exit(EXIT_FAILURE);
        case Parameter:
            fprintf(stderr, "Parameter expression outside of function definition\n");
            exit(EXIT_FAILURE);
        default: {
            char *expr_str = expr_tree_to_str(tree);
            fprintf(stderr, "Failed to evaluate unrecognized expression: %s\n", expr_str);
            free(expr_str);
            exit(EXIT_FAILURE);
        }
    }
}

static ExprTree *eval_fun(ExprTree *tree, Env_t *env) {
    ExprTree *body = eval(tree->right, env);
    tree->right = body;
    extend_env(&env, tree->value.id, tree);

    return tree;
}

static ExprTree *eval_binop(ExprTree *tree, Env_t *env) {
    ExprTree *v1, *v2, *v;
    int can_simplify, v1_is_int, v1_is_float, v1_is_int, v2_is_float;

    v1 = eval(tree->left, env);
    v2 = eval(tree->right, env);

    v1_is_int = v1->expr == Int;
    v1_is_float = v1->expr == Float;
    v2_is_int = v2->expr == Int;
    v2_is_float = v2->expr == Float;
    can_simplify = (v1_is_int || v1_is_float) && (v2_is_int || v2_is_float);

    if (can_simplify) {
        v = malloc(sizeof(ExprTree));
        v->expr = (v1_is_float || v2_is_float) ? Float : Int;
        v->left = NULL;
        v->right = NULL;
    } else {
        tree->left = v1;
        tree->right = v2;

        return tree;
    }

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
                if (v1 % v2 == 0) {
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
    return v;
}

static ExprTree *eval_assign(ExprTree *tree, Env_t **env) {
    ExprTree *v = eval(tree->right);
    tree->right = v;
    extend_env(env, tree->left->value.id, v);

    return tree;
}

static ExprTree *eval_application(ExprTree *tree, Env_t *env) {
    ExprTree *fun = eval(tree->left, env);
    ExprTree *fun_body = fun->right;
    ExprTree *params = fun->left;
    ExprTree *args = eval_arguments(tree->right, params, env);
    ExprTree *ret_val;
    int num_params;

    num_params = push_params(params, env);
    bind_args(args, env);
    ret_val = eval(fun_body);
    pop_params(params, env);

    return ret_val;
}

static int push_params(ExprTree *tree, Env_t *env) {
    if (tree == NULL) {
        return 0;
    }

    extend_env_tmp(&env, tree->left->value.id);

    return 1 + push_params(tree->right, env);
}

static void pop_params(ExprTree *tree, int num_params, Env_t *env) {}
static ExprTree *eval_arguments(ExprTree *tree, ExprTree *fun_params, Env_t *env) {return NULL;}
