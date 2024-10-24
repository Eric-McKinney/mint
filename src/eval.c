#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <math.h>
#include <limits.h>
#include "eval.h"
#include "parser.h"

#define MAX_PARAMS 50       /* arbitrary upper limit on how many params a function can have */
#define MAX_NODE_VAL_LEN 50 /* arbitrary upper limit on value length for a node in chars (i.e. an ID or float) */

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
        case Fun:
            copy->value.id = malloc(strlen(tree->value.id) + 1);
            strcpy(copy->value.id, tree->value.id);
            break;
        case Int:
        case Float:
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

    if (errno != 0) {
        return;
    }

    new_data = malloc(sizeof(Env_t));
    new_data->id = malloc(strlen(id) + 1);
    strcpy(new_data->id, id);
    new_data->data = copy_expr_tree(data);
    new_data->next = env->next;

    env->next = new_data;
}

static void extend_env_tmp(Env_t *env, const char *id) {
    ExprTree *dummy_data = calloc(1, sizeof(ExprTree));

    if (errno != 0) {
        free(dummy_data);
        return;
    }

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
        errno = EINVAL;
        warnx("error: Unbound identifier: %s\n", id);
        return NULL;
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
        errno = EINVAL;
        warnx("error: Failed to shrink environment: cannot find identifier %s\n", id);
        return;
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

    if (errno != 0) {
        return;
    }

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

static ExprTree *eval_expr(ExprTree **tree, Env_t *env, char in_fun);
static void eval_fun(ExprTree **t, Env_t *env);
static void eval_binop(ExprTree **t, Env_t *env, char in_fun);
static void eval_assign(ExprTree *tree, Env_t *env);
static void eval_application(ExprTree **t, Env_t *env, char in_fun);
static void eval_arguments(ExprTree **t, Env_t *env, char in_fun);
static int push_params(ExprTree *params, Env_t *env);
static int bind_args(ExprTree *args, ExprTree *params, Env_t *env);
static void pop_params(ExprTree *params, int num_params, Env_t *env);

ExprTree *eval(ExprTree **tree, Env_t *env) {
    return eval_expr(tree, env, 0);
}

static ExprTree *eval_expr(ExprTree **tree, Env_t *env, char in_fun) {
    if (*tree == NULL) {
        return NULL;
    }

    switch ((*tree)->expr) {
        case Int:
        case Float:
            return *tree;
        case ID: {
            if (!in_fun) {
                ExprTree *value = lookup(env, (*tree)->value.id);
                free_expr_tree(*tree);
                *tree = value;
            }

            break;
        }
        case Fun:
            eval_fun(tree, env);
            break;
        case Binop:
            eval_binop(tree, env, in_fun);
            break;
        case Assign:
            eval_assign(*tree, env);
            break;
        case Application:
            eval_application(tree, env, in_fun);
            break;
        case Argument:
            errno = EINVAL;
            warnx("error: Argument expression outside of function application\n");
            return NULL;
        case Parameter:
            errno = EINVAL;
            warnx("error: Parameter expression outside of function definition\n");
            return NULL;
        default: {
            char *expr_str = expr_tree_to_str(*tree);
            errno = EINVAL;
            warnx("error: Failed to evaluate unrecognized expression: %s\n", expr_str);
            free(expr_str);
            return NULL;
        }
    }
    
    return *tree;
}

static int validate_params(ExprTree *params, const char *fun_id) {
    char *p[MAX_PARAMS];
    int i = 0, j, dupes = 0;

    while (params != NULL) {
        p[i] = params->left->value.id;

        for (j = 0; j < i; j++) {
            if (strcmp(p[j], p[i]) == 0) {
                errno = EINVAL;
                warnx("Duplicate parameter \"%s\" in %s\n", p[i], fun_id);
                dupes++;
                break;
            }
        }

        i++;
        params = params->right;
    }

    if (dupes) {
        return -1;
    }

    return 0;
}

static void eval_fun(ExprTree **t, Env_t *env) {
    ExprTree *tree = *t;
    int num_params;

    num_params = push_params(tree->left, env);
    eval_expr(&(tree->right), env, 1);

    if (validate_params(tree->left, tree->value.id) == 0) {
        extend_env(env, tree->value.id, tree);
    }

    pop_params(tree->left, num_params, env);
}

/*
 * Checks binary operation for integer overflow or underflow.
 * Returns 1 if overflow is detected, 2 if underflow is detected, and 0 otherwise.
 */
static int check_int_limits(long v1, long v2, Operator_t op) {
    int overflow = 1, underflow = 2, op_is_ok = 0;

    switch(op) {
        case Add: {
            int both_pos = v1 > 0 && v2 > 0;
            int both_neg = v1 < 0 && v2 < 0;

            if (both_pos && v1 > LONG_MAX - v2) {
                return overflow;
            }

            if (both_neg && v1 < LONG_MIN + v2) {
                return underflow;
            }

            return op_is_ok;
        }
        case Sub: {
            int pos_neg = v1 > 0 && v2 < 0;
            int neg_pos = v1 < 0 && v2 > 0;

            if (pos_neg && v1 > LONG_MAX + v2) {
                return overflow;
            }

            if (neg_pos && v1 < LONG_MIN + v2) {
                return underflow;
            }

            return op_is_ok;
        }
        case Mult: {
            int same_sign = (v1 > 0 && v2 > 0) || (v1 < 0 && v2 < 0);
            int diff_sign = !same_sign;
            /* I tried using fabs, but casting back and forth loses precision */
            v1 = v1 >= 0 ? v1 : -1 * v1;  
            v2 = v2 >= 0 ? v2 : -1 * v2;

            if (same_sign && v1 > LONG_MAX / v2) {
                return overflow;
            }

            if (diff_sign && -v1 < LONG_MIN / v2) {
                return underflow;
            }

            return op_is_ok;
        }
        case Exp: {
            int both_pos = v1 > 0 && v2 > 0;

            if (both_pos && v1 >= pow(LONG_MAX, 1.0/v2)) {
                return overflow;
            }

            return op_is_ok;
        }
        default:
            return op_is_ok;
    }
}

static int check_float_limits(double v1, double v2, Operator_t op) {
    return 0;
}

static void interpret_limit_check(int check_result, int is_int) {
    char i[] = "integer";
    char d[] = "float";
    char *type = (is_int ? i : d);

    switch(check_result) {
        case 1:
            errno = ERANGE;
            warnx("error: %s overflow detected", type);
            return;
        case 2:
            errno = ERANGE;
            warnx("error: %s underflow detected", type);
            return;
        case 0:
            return;
        default:
            errno = EINVAL;
            warnx("unexpected return value while checking %s limits for binop", type);
            return;
    }
}

static void eval_binop(ExprTree **t, Env_t *env, char in_fun) {
    ExprTree *tree = *t;
    ExprTree *v1, *v2, *v;
    int can_simplify, v1_is_int, v1_is_float, v2_is_int, v2_is_float;

    v1 = eval_expr(&(tree->left), env, in_fun);
    v2 = eval_expr(&(tree->right), env, in_fun);

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

    if (v->expr == Float) {
        if (v1_is_int) {
            v1->value.d = (double) v1->value.i;
        } else if (v2_is_int) {
            v2->value.d = (double) v2->value.i;
        }
    }

    switch (v->expr) {
        case Int:
            interpret_limit_check(check_int_limits(v1->value.i, v2->value.i, tree->value.binop), 1);
            break;
        case Float:
            interpret_limit_check(check_float_limits(v1->value.d, v2->value.d, tree->value.binop), 0);
            break;
        default:
            errno = EINVAL;
            warnx("unexpected expression type while checking for over/underflow");
            return;
    }

    switch (tree->value.binop) {
        case Add:
            if (v->expr == Int) {
                v->value.i = v1->value.i + v2->value.i;
            } else {
                v->value.d = v1->value.d + v2->value.d;
            }

            break;
        case Sub:
            if (v->expr == Int) {
                v->value.i = v1->value.i - v2->value.i;
            } else {
                v->value.d = v1->value.d - v2->value.d;
            }

            break;
        case Mult:
            if (v->expr == Int) {
                v->value.i = v1->value.i * v2->value.i;
            } else {
                v->value.d = v1->value.d * v2->value.d;
            }

            break;
        case Div:
            if ((v->expr == Int ? v2->value.i : v2->value.d) == 0) {
                errno = EINVAL;
                warnx("error: division by 0");
                return;
            }

            if (v->expr == Int) {
                if (v1->value.i % v2->value.i == 0) {
                    v->value.i = v1->value.i / v2->value.i;
                } else {
                    v->expr = Float;
                    v->value.d = (double) v1->value.i / (double) v2->value.i;
                }
            } else {
                v->value.d = v1->value.d / v2->value.d;
            }
            
            break;
        case Exp:
            if (v->expr == Int) {
                v1->value.d = (double) v1->value.i;
                v2->value.d = (double) v2->value.i;

                v->value.i = (long int) pow(v1->value.d, v2->value.d);
            } else {
                v->value.d = pow(v1->value.d, v2->value.d);
            }

            break;
        default: {
            char *expr_str = expr_tree_to_str(tree);
            errno = EINVAL;
            warnx("error: Failed to evaluate unrecognized binary operator expression: %s\n", expr_str);
            free(expr_str);
            return;
        }
    }

    free_expr_tree(tree);
    *t = v;
}

static void eval_assign(ExprTree *tree, Env_t *env) {
    ExprTree *value = eval(&(tree->right), env);
    
    if (env_find(env, tree->left->value.id, NULL) != NULL) {
        update_env(env, tree->left->value.id, value);
    } else {
        extend_env(env, tree->left->value.id, value);
    }
}

static void eval_application(ExprTree **t, Env_t *env, char in_fun) {
    ExprTree *tree = *t;
    ExprTree *fun = lookup(env, tree->left->value.id);
    ExprTree *params, *fun_body, *ret_val;
    ExprTree *args = tree->right;
    int num_params, num_args_bound;
    
    /* lookup failed because the function is not in env */
    if (fun == NULL) {
        /* errno already set by lookup, but just to be clear there's an error */
        errno = EINVAL;
        return;
    }

    params = fun->left;
    fun_body = fun->right;

    eval_arguments(&(args), env, in_fun);
    num_params = push_params(params, env);
    num_args_bound = bind_args(args, params, env);

    if (num_params != num_args_bound) {
        errno = EINVAL;
        warnx("In application of %s: received %d arguments, expected %d\n", fun->value.id, num_args_bound, num_params);
        pop_params(params, num_params, env);
        return;
    }

    if (in_fun) {
        pop_params(params, num_params, env);
        return;
    }

    ret_val = eval(&fun_body, env);
    pop_params(params, num_params, env);

    free_expr_tree(tree);

    fun->right = NULL;  /* fun->right is currently the ret_val & don't want to free it yet  */
    free_expr_tree(fun);

    *t = ret_val;
}

static void eval_arguments(ExprTree **t, Env_t *env, char in_fun) {
    ExprTree *arg = *t;

    while (arg != NULL) {
        eval_expr(&(arg->left), env, in_fun);
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
    int num_args;

    if (args == NULL) {
        return 0;
    }

    if (params == NULL) {
        return 1 + bind_args(args->right, params, env);
    }

    num_args = bind_args(args->right, params->right, env);
    update_env(env, params->left->value.id, args->left);

    return 1 + num_args;
}

static void pop_params(ExprTree *params, int num_params, Env_t *env) {
    char *top_param; /* first param in env (top of stack) */

    while (params->right != NULL) {
        params = params->right;
    }

    top_param = params->left->value.id;
    shrink_env(env, top_param, num_params);
}

static int is_primary_expr(ExprTree *tree) {
    if (tree == NULL) {
        return 0;
    }

    return tree->expr == Int || tree->expr == Float || tree->expr == ID;
}

static char *eval_result_to_str_aux(ExprTree *tree, int *size) {
    char *str, *lstr, *rstr;
    int lsize, rsize;

    if (tree == NULL) {
        str = calloc(1, 1);
        *size = 0;
        return str;
    }

    if (tree->left != NULL || tree->right != NULL) {
        lstr = eval_result_to_str_aux(tree->left, &lsize);
        rstr = eval_result_to_str_aux(tree->right, &rsize);
    } else {
        lstr = NULL;
        rstr = NULL;
        lsize = 0;
        rsize = 0;
    }

    str = malloc(MAX_NODE_VAL_LEN + lsize + rsize + 1);

    switch (tree->expr) {
        case Int:
            sprintf(str, "%ld", tree->value.i);
            break;
        case Float:
            sprintf(str, "%f", tree->value.d);
            break;
        case ID:
            strcpy(str, tree->value.id);
            break;
        case Fun:
            sprintf(str, "%s(%s) = %s", tree->value.id, lstr, rstr);
            break;
        case Binop:
            switch (tree->value.binop) {
                case Add:
                    sprintf(str, "%s + %s", lstr, rstr);
                    break;
                case Sub:
                    sprintf(str, "%s - %s", lstr, rstr);
                    break;
                case Mult:
                    sprintf(str, "%s * %s", lstr, rstr);
                    break;
                case Div: {
                    char lexp[MAX_NODE_VAL_LEN], rexp[MAX_NODE_VAL_LEN];

                    if (!is_primary_expr(tree->left)) {
                        sprintf(lexp, "(%s)", lstr);
                    } else {
                        strcpy(lexp, lstr);
                    }

                    if (!is_primary_expr(tree->right)) {
                        sprintf(rexp, "(%s)", rstr);
                    } else {
                        strcpy(rexp, rstr);
                    }

                    sprintf(str, "%s / %s", lexp, rexp);
                    break;
                }
                case Exp: {
                    char lexp[MAX_NODE_VAL_LEN], rexp[MAX_NODE_VAL_LEN];

                    if (!is_primary_expr(tree->left)) {
                        sprintf(lexp, "(%s)", lstr);
                    } else {
                        strcpy(lexp, lstr);
                    }

                    if (!is_primary_expr(tree->right)) {
                        sprintf(rexp, "(%s)", rstr);
                    } else {
                        strcpy(rexp, rstr);
                    }

                    sprintf(str, "%s^%s", lstr, rstr);
                    break;
                }
                default:
                    sprintf(str, "%s unknown-op %s", lstr, rstr);
            }
            break;
        case Assign:
            sprintf(str, "%s: %s", lstr, rstr);
            break;
        case Application:
            sprintf(str, "%s(%s)", lstr, rstr);
            break;
        case Argument:
        case Parameter:
            if (rsize == 0) {
                sprintf(str, "%s", lstr);
            } else {
                sprintf(str, "%s, %s", lstr, rstr);
            }

            break;
        default:
            strcpy(str, "Unrecognized expr");
    }

    if (tree->left != NULL || tree->right != NULL) {
        free(lstr);
        free(rstr);
    }

    *size = strlen(str);
    return str;
}

char *eval_result_to_str(ExprTree *tree) {
    int size;
    return eval_result_to_str_aux(tree, &size);
}

