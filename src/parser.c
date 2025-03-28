#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include "parser.h"
#include "lexer.h"

#define MAX_NODE_LEN 6      /* longest node name is Assign = 6 chars                           */
#define MAX_NODE_VAL_LEN 50 /* arbitrary upper limit on node value size (e.g. a variable name) */
#define MAX_NODE_STR_LEN MAX_NODE_LEN + MAX_NODE_VAL_LEN

static TokenList *match_token(TokenList *tok_l, Tok_t tok) {
    char *expected, *input, *arg;
    
    if (errno != 0) {
        return NULL;
    }
    if (tok_l != NULL && tok_l->token == tok) {
        return tok_l->next;
    }

    switch (tok) {
        case TOK_ID:
            expected = malloc(6);
            strcpy(expected, "an ID");
            break;
        case TOK_INT:
        case TOK_FLOAT:
            expected = malloc(9);
            strcpy(expected, "a number");
            break;
        default: {
            TokenList t = {0};
            t.token = tok;

            expected = token_value_to_str(&t);
        }
    }

    input = token_values_to_str(tok_l);
    arg = token_value_to_str(tok_l);

    errno = EINVAL;
    warnx("error: (E1001) expected %s from remaining input \"%s\", but got \"%s\" instead", expected, input, arg);
    
    free(expected);
    free(input);
    free(arg);

    return NULL;
}

static ExprTree *parse_input(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_function_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_parameter_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_assignment_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_additive_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_multiplicative_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_exponent_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_application_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_arg_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_primary_expr(TokenList *tok_l, TokenList **out_tl);

ExprTree *parse(TokenList *tok_l) {
    TokenList *t;
    return parse_input(tok_l, &t);
}

/*
 * Input -> Expr \n | Comment \n | Expr Comment \n
 */
static ExprTree *parse_input(TokenList *tok_l, TokenList **out_tl) {
    ExprTree *expr = NULL;
    TokenList *t = tok_l;

    if (tok_l == NULL) {
        return NULL;
    }

    if (tok_l->token != TOK_COMMENT) {
        expr = parse_expr(tok_l, &t);
    } else {
        t = tok_l;
    }

    if (t != NULL && t->token == TOK_COMMENT) {
        t = match_token(t, TOK_COMMENT);
    }

    if (errno == 0 && t != NULL) {
        char *tok_l_str = token_values_to_str(t);

        errno = EINVAL;
        warnx("error: (E1002) parsing ended early with remaining tokens: \"%s\"", tok_l_str);

        free(tok_l_str);
    }
    
    return expr;
}

/*
 * Expr -> FunctionExpr | AssignmentExpr | AdditiveExpr
 */
static ExprTree *parse_expr(TokenList *tok_l, TokenList **out_tl) {
    ExprTree *add_expr;
    TokenList *t;

    if (errno != 0) {
        return NULL;
    }

    switch (tok_l->token) {
        case TOK_FUN:
            return parse_function_expr(tok_l, out_tl);
        case TOK_ID:
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
            /* The above is to ignore a warning for implicit fall through caused by this case */
            if (tok_l->next != NULL && tok_l->next->token == TOK_EQUAL) {
                return parse_assignment_expr(tok_l, out_tl);
            }
            #pragma GCC diagnostic pop
        default:
            add_expr = parse_additive_expr(tok_l, &t);

            *out_tl = t;
            return add_expr;
    }
}

/*
 * FunctionExpr -> fn ID(ParamExpr) = AdditiveExpr \n
 */
static ExprTree *parse_function_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3, *t4, *t5, *t6, *t7;
    ExprTree *fun_expr, *param_expr, *body_expr;
    char *id, *id_cpy;

    if (errno != 0) {
        return NULL;
    }

    t = match_token(tok_l, TOK_FUN);
    t2 = match_token(t, TOK_ID);

    if (errno != 0) {
        *out_tl = NULL;
        return NULL;
    }

    id = t->value.id;
    id_cpy = malloc(strlen(id) + 1);
    strcpy(id_cpy, id);

    t3 = match_token(t2, TOK_LPAREN);

    param_expr = parse_parameter_expr(t3, &t4);

    if (errno != 0) {
        free(id_cpy);
        free_expr_tree(param_expr);
        return NULL;
    }

    t5 = match_token(t4, TOK_RPAREN);
    t6 = match_token(t5, TOK_EQUAL);

    body_expr = parse_additive_expr(t6, &t7);

    fun_expr = malloc(sizeof(ExprTree));
    fun_expr->expr = Fun;
    fun_expr->value.id = id_cpy;
    fun_expr->left = param_expr;
    fun_expr->right = body_expr;

    *out_tl = t7;
    return fun_expr;
}

/*
 * ParameterExpr -> ID, ParamExpr | ID
 */
static ExprTree *parse_parameter_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *parameter_expr, *param_expr, *id_expr;

    if (errno != 0) {
        return NULL;
    }

    if (tok_l != NULL && tok_l->token != TOK_ID) {
        char *tok_str = token_value_to_str(tok_l);

        errno = EINVAL;
        warnx("error: (E1003) malformed parameter \"%s\"", tok_str);

        free(tok_str);
        *out_tl = NULL;
        return NULL;
    }

    id_expr = parse_primary_expr(tok_l, &t);

    if (errno != 0) {
        free_expr_tree(id_expr);
        return NULL;
    }

    if (t != NULL && t->token == TOK_COMMA) {
        t2 = match_token(t, TOK_COMMA);
        param_expr = parse_parameter_expr(t2, &t3);

        *out_tl = t3;
    } else {
        param_expr = NULL;
        
        *out_tl = t;
    }

    parameter_expr = malloc(sizeof(ExprTree));
    parameter_expr->expr = Parameter;
    parameter_expr->left = id_expr;
    parameter_expr->right = param_expr;

    return parameter_expr;
}

/*
 * AssignmentExpr -> ID = AdditiveExpr \n
 */
static ExprTree *parse_assignment_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *assign_expr, *id_expr, *val_expr;

    if (errno != 0) {
        return NULL;
    }

    id_expr = parse_primary_expr(tok_l, &t);
    t2 = match_token(t, TOK_EQUAL);
    val_expr = parse_additive_expr(t2, &t3);

    assign_expr = malloc(sizeof(ExprTree));
    assign_expr->expr = Assign;
    assign_expr->left = id_expr;
    assign_expr->right = val_expr;

    *out_tl = t3;
    return assign_expr;
}

/*
 * AdditiveExpr -> AdditiveExpr AdditiveOperator MultiplicativeExpr | MultiplicativeExpr
 * AdditiveOperator -> + | -
 */
static ExprTree *parse_additive_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2;
    ExprTree *additive_expr, *add_expr, *mult_expr;

    if (errno != 0) {
        return NULL;
    }

    mult_expr = parse_multiplicative_expr(tok_l, &t);

    if (t == NULL || (t->token != TOK_ADD && t->token != TOK_SUB)) {
        *out_tl = t;
        return mult_expr;
    }

    add_expr = mult_expr;
    while (t != NULL && (t->token == TOK_ADD || t->token == TOK_SUB)) {
        Tok_t op = t->token;

        t2 = match_token(t, op);
        mult_expr = parse_multiplicative_expr(t2, &t);

        additive_expr = malloc(sizeof(ExprTree));
        additive_expr->expr = Binop;
        additive_expr->value.binop = (op == TOK_ADD) ? Add : Sub;
        additive_expr->left = add_expr;
        additive_expr->right = mult_expr;

        add_expr = additive_expr;
    }

    *out_tl = t;
    return additive_expr;
}

/*
 * MultiplicativeExpr -> MultiplicativeExpr MultiplicativeOperator ApplicationExpr | ApplicationExpr
 * MultiplicativeOperator -> * | /
 */
static ExprTree *parse_multiplicative_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2;
    ExprTree *multiplicative_expr, *mult_expr, *exp_expr;

    if (errno != 0) {
        return NULL;
    }

    exp_expr = parse_exponent_expr(tok_l, &t);

    if (t == NULL || (t->token != TOK_MULT && t->token != TOK_DIV)) {
        *out_tl = t;
        return exp_expr;
    }

    mult_expr = exp_expr;
    while (t != NULL && (t->token == TOK_MULT || t->token == TOK_DIV)) {
        Tok_t op = t->token;

        t2 = match_token(t, op);
        exp_expr = parse_exponent_expr(t2, &t);

        multiplicative_expr = malloc(sizeof(ExprTree));
        multiplicative_expr->expr = Binop;
        multiplicative_expr->value.binop = (op == TOK_MULT) ? Mult : Div;
        multiplicative_expr->left = mult_expr;
        multiplicative_expr->right = exp_expr;

        mult_expr = multiplicative_expr;
    }

    *out_tl = t;
    return multiplicative_expr;
}

/*
 * ExponentExpr -> ApplicationExpr ^ ApplicationExpr | ApplicationExpr
 */
static ExprTree *parse_exponent_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *exponent_expr, *app_expr1, *app_expr2;

    if (errno != 0) {
        return NULL;
    }

    app_expr1 = parse_application_expr(tok_l, &t);

    if (t == NULL || t->token != TOK_EXP) {
        *out_tl = t;
        return app_expr1;
    }

    t2 = match_token(t, TOK_EXP);
    app_expr2 = parse_application_expr(t2, &t3);

    exponent_expr = malloc(sizeof(ExprTree));
    exponent_expr->expr = Binop;
    exponent_expr->value.binop = Exp;
    exponent_expr->left = app_expr1;
    exponent_expr->right = app_expr2;

    *out_tl = t3;
    return exponent_expr;
}

/*
 * ApplicationExpr -> ID(ArgExpr) | PrimaryExpr
 */
static ExprTree *parse_application_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3, *t4;
    ExprTree *application_expr, *primary_expr, *id_expr, *arg_expr;

    if (errno != 0) {
        return NULL;
    }

    if (tok_l == NULL) {
        errno = EINVAL;
        warnx("error: (E1004) incomplete expression");

        *out_tl = NULL;
        return NULL;
    }

    switch (tok_l->token) {
        case TOK_ID:
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
            /* ignoring warning because IDs can mean fn application (so break) or just a variable (so fallthrough) */
            if (tok_l->next != NULL && tok_l->next->token == TOK_LPAREN) {
                break;
            }
            #pragma GCC diagnostic pop
        case TOK_INT:
        case TOK_FLOAT:
        case TOK_LPAREN:
            primary_expr = parse_primary_expr(tok_l, &t);

            *out_tl = t;
            return primary_expr;
        default: {
            char *tok_l_str = token_values_to_str(tok_l);

            errno = EINVAL;
            warnx("error: (E1006) failed to find ID or number with remaining input: \"%s\"", tok_l_str);
            free(tok_l_str);

            *out_tl = NULL;
            return NULL;
        }
    }
    

    id_expr = parse_primary_expr(tok_l, &t);
    t2 = match_token(t, TOK_LPAREN);
    arg_expr = parse_arg_expr(t2, &t3);
    t4 = match_token(t3, TOK_RPAREN);

    application_expr = malloc(sizeof(ExprTree));
    application_expr->expr = Application;
    application_expr->left = id_expr;
    application_expr->right = arg_expr;

    *out_tl = t4;
    return application_expr;
}

/*
 * ArgExpr -> AdditiveExpr, ArgExpr | AdditiveExpr
 */
static ExprTree *parse_arg_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *argument_expr, *add_expr, *arg_expr;

    if (errno != 0) {
        return NULL;
    }

    add_expr = parse_additive_expr(tok_l, &t);

    if (errno != 0) {
        free_expr_tree(add_expr);

        *out_tl = NULL;
        return NULL;
    }

    if (t != NULL && t->token == TOK_COMMA) {
        t2 = match_token(t, TOK_COMMA);
        arg_expr = parse_arg_expr(t2, &t3);
        
        *out_tl = t3;
    } else {
        arg_expr = NULL;

        *out_tl = t;
    }

    argument_expr = malloc(sizeof(ExprTree));
    argument_expr->expr = Argument;
    argument_expr->left = add_expr;
    argument_expr->right = arg_expr;

    return argument_expr;
}

/*
 * PrimaryExpr -> int | float | ID | (AdditiveExpr)
 * ID -> string which matches the following regex: ^[a-zA-Z][a-zA-Z0-9_]*$
 */
static ExprTree *parse_primary_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *p_expr, *add_expr;
    char *id, *id_cpy;

    if (errno != 0) {
        return NULL;
    }

    if (tok_l == NULL) {
        errno = EINVAL;
        warnx("error: (E1007) input ended before expected");

        *out_tl = NULL;
        return NULL;
    }

    p_expr = malloc(sizeof(ExprTree));

    switch (tok_l->token) {
        case TOK_INT:
            t = match_token(tok_l, TOK_INT);

            p_expr->expr = Int;
            p_expr->value.i = tok_l->value.i;
            break;
        case TOK_FLOAT:
            t = match_token(tok_l, TOK_FLOAT);

            p_expr->expr = Float;
            p_expr->value.d = tok_l->value.d;
            break;
        case TOK_ID:
            t = match_token(tok_l, TOK_ID);

            id = tok_l->value.id;
            id_cpy = malloc(strlen(id) + 1);
            strcpy(id_cpy, id);

            p_expr->expr = ID;
            p_expr->value.id = id_cpy;
            break;
        case TOK_LPAREN:
            t = match_token(tok_l, TOK_LPAREN);
            add_expr = parse_additive_expr(t, &t2);
            t3 = match_token(t2, TOK_RPAREN);

            free(p_expr);
            *out_tl = t3;
            return add_expr;
        default: {
            char *tok_l_str = token_values_to_str(tok_l);
            free(p_expr);

            errno = EINVAL;
            warnx("error: (E1008) unrecognized primary expression with remaining tokens:\"%s\"", tok_l_str);
            free(tok_l_str);

            *out_tl = NULL;
            return NULL;
        }
    }

    p_expr->left = NULL;
    p_expr->right = NULL;

    *out_tl = t;
    return p_expr;
}

static void free_tree_node(ExprTree *tree) {
    if (tree->expr == ID || tree->expr == Fun) {
        free(tree->value.id);
    }

    free(tree);
}

void free_expr_tree(ExprTree *tree) {
    if (tree == NULL) {
        return;
    }
    
    free_expr_tree(tree->left);
    free_expr_tree(tree->right);
    free_tree_node(tree);
}

static char *expr_tree_to_str_aux(ExprTree *tree, int *size) {
    char *str, *lstr, *rstr;
    int lsize, rsize;

    if (tree == NULL) {
        str = malloc(3);
        strcpy(str, "()");

        *size = 0;
        return str;
    }

    if (tree->left != NULL || tree->right != NULL) {
        lstr = expr_tree_to_str_aux(tree->left, &lsize);
        rstr = expr_tree_to_str_aux(tree->right, &rsize);
    } else {
        lsize = 0;
        rsize = 0;
    }

    /* +2 for parens () and then +1 for null terminator */
    str = malloc(MAX_NODE_STR_LEN*(1 + lsize + rsize) + 3);

    strcpy(str, "(");
    switch (tree->expr) {
        char s[MAX_NODE_VAL_LEN];

        case Int:
            sprintf(s, "Int %ld", tree->value.i);
            strcat(str, s);
            break;
        case Float:
            sprintf(s, "Float %f", tree->value.d);
            strcat(str, s);
            break;
        case ID:
            sprintf(s, "ID %s", tree->value.id);
            strcat(str, s);
            break;
        case Fun:
            sprintf(s, "Fun %s ", tree->value.id);
            strcat(str, s);
            break;
        case Binop:
            switch (tree->value.binop) {
                case Add:
                    strcat(str, "Add");
                    break;
                case Sub:
                    strcat(str, "Sub");
                    break;
                case Mult:
                    strcat(str, "Mult");
                    break;
                case Div:
                    strcat(str, "Div");
                    break;
                case Exp:
                    strcat(str, "Exp");
                    break;
                default:
                    strcat(str, "Unrecognized binop");
            }
            break;
        case Assign:
            strcat(str, "Assign");
            break;
        case Application:
            strcat(str, "App");
            break;
        case Argument:
            strcat(str, "Arg");
            break;
        case Parameter:
            strcat(str, "Param");
            break;
        default:
            strcat(str, "Unrecognized expr");
    }

    if (tree->left != NULL || tree->right != NULL) {
        strcat(str, lstr);
        strcat(str, rstr);

        free(lstr);
        free(rstr);
    }
    
    strcat(str, ")");

    *size = 1 + lsize + rsize;
    return str;
}

char *expr_tree_to_str(ExprTree *tree) {
    int size;
    return expr_tree_to_str_aux(tree, &size);
}
