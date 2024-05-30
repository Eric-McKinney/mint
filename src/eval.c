#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eval.h"
#include "parser.h"

Env_t *init_env() {
    return NULL;
}

static void extend_env(Env_t **env, const char *id, ExprTree *data) {
    Env_t *new_data;

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = data;
    new_data->next = *env;

    *env = new_data;
}

static Env_t *env_find(Env_t *env, const char *id, Env_t **prev) {
    Env_t *p = NULL;

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
        fprintf(stderr, "Unbound variable %s", id);
        exit(EXIT_FAILURE);
    }
}

static void free_env_node(Env_t *env) {
    free(env->id);
    free(env);
}

static void shrink_env(Env_t **env, const char *id) {
    Env_t *prev;
    Env_t *e = env_find(*env, id, &prev);

    if (e != NULL) {
        if (prev == NULL) {
            *env = e->next;
        } else {
            prev->next = e->next;
        }

        free_env_node(e);
    }
}

static void extend_env_tmp(Env_t **env, const char *id) {
    extend_env(env, id, NULL);
}

static void update_env(Env_t *env, const char *id, ExprTree *data) {
    ExprTree **old_data = &lookup(env, id);

    *old_data = data;
}

void free_env(Env_t *env) {
    if (env == NULL) {
        return;
    }

    free_env(env->next);
    free_env_node(env);
}

static ExprTree *eval_fun(ExprTree *tree, Env_t *env);
static ExprTree *eval_binop(ExprTree *tree, Env_t *env);
static ExprTree *eval_assign(ExprTree *tree, Env_t *env);
static ExprTree *eval_application(ExprTree *tree, Env_t *env);
static ExprTree *eval_argument(ExprTree *tree, Env_t *env);
static ExprTree *eval_parameter(ExprTree *tree, Env_t *env);

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

static ExprTree *eval_application(ExprTree *tree, Env_t *env) {return NULL;}
static ExprTree *eval_argument(ExprTree *tree, Env_t *env) {return NULL;}
static ExprTree *eval_parameter(ExprTree *tree, Env_t *env) {return NULL;}
