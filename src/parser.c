#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

static TokenList *match_token(TokenList *tok_l, Tok_t tok) {
    TokenList expected_token = {0}; /* only for token_to_str call below */
    char *expected, *input, *arg;
    
    if (tok_l->token == tok) {
        return tok_l->next;
    }

    expected_token.token = tok;

    expected = token_to_str(&expected_token);
    input = token_list_to_str(tok_l);
    arg = token_to_str(tok_l);

    fprintf(stderr, "Expected %s from input %s, got %s\n", expected, input, arg);
    
    free(expected);
    free(input);
    free(arg);

    exit(EXIT_FAILURE);
}

static ExprTree *parse_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_function_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_parameter_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_assignment_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_additive_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_multiplicative_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_application_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_arg_expr(TokenList *tok_l, TokenList **out_tl);
static ExprTree *parse_primary_expr(TokenList *tok_l, TokenList **out_tl);

ExprTree *parse(TokenList *tok_l) {
    TokenList *t;
    return parse_expr(tok_l, &t);
}

static ExprTree *parse_expr(TokenList *tok_l, TokenList **out_tl) {
    ExprTree *add_expr;
    TokenList *t;

    if (tok_l == NULL) {
        fprintf(stderr, "Empty input\n");
        exit(EXIT_FAILURE);
    }

    switch(tok_l->token) {
        case TOK_FUN:
            return parse_function_expr(tok_l, out_tl);
        case TOK_ID:
            return parse_assignment_expr(tok_l, out_tl);
        default:
            add_expr = parse_additive_expr(tok_l, &t);

            *out_tl = match_token(t, TOK_ENDLN);
            return add_expr;
    }
}

static ExprTree *parse_function_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3, *t4, *t5, *t6, *t7, *t8;
    ExprTree *fun_expr, *param_expr, *body_expr;
    char *id, *id_cpy;

    t = match_token(tok_l, TOK_FUN);
    t2 = match_token(t, TOK_ID);

    id = t->value.id;
    id_cpy = malloc(strlen(id) + 1);
    strcpy(id_cpy, id);

    t3 = match_token(t2, TOK_LPAREN);

    param_expr = parse_parameter_expr(t3, &t4);

    t5 = match_token(t4, TOK_RPAREN);
    t6 = match_token(t5, TOK_EQUAL);

    body_expr = parse_additive_expr(t6, &t7);

    t8 = match_token(t7, TOK_ENDLN);

    fun_expr = malloc(sizeof(ExprTree));
    fun_expr->expr = Fun;
    fun_expr->value.id = id_cpy;
    fun_expr->left = param_expr;
    fun_expr->right = body_expr;

    *out_tl = t8;
    return fun_expr;
}

static ExprTree *parse_parameter_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *param_expr;
    char *id, *id_cpy;

    t = match_token(tok_l, TOK_ID);

    id = tok_l->value.id;
    id_cpy = malloc(strlen(id) + 1);
    strcpy(id_cpy, id);

    param_expr = malloc(sizeof(ExprTree));
    param_expr->expr = ID;
    param_expr->value.id = id_cpy;
    param_expr->left = NULL;
    param_expr->right = NULL;

    if (t->token == TOK_COMMA) {
        t2 = match_token(t, TOK_COMMA);

        param_expr->right = parse_parameter_expr(t2, &t3);

        *out_tl = t3;
    } else {
        *out_tl = t;
    }

    return param_expr;
}

static ExprTree *parse_assignment_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3, *t4;
    ExprTree *assign_expr, *val_expr;
    char *id, *id_cpy;

    t = match_token(tok_l, TOK_ID);

    id = tok_l->value.id;
    id_cpy = malloc(strlen(id) + 1);
    strcpy(id_cpy, id);

    t2 = match_token(t, TOK_EQUAL);

    val_expr = parse_additive_expr(t2, &t3);

    t4 = match_token(t3, TOK_ENDLN);

    assign_expr = malloc(sizeof(ExprTree));
    assign_expr->expr = Assign;
    assign_expr->value.id = id_cpy;
    assign_expr->left = NULL;
    assign_expr->right = val_expr;

    *out_tl = t4;
    return assign_expr;
}

static ExprTree *parse_additive_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    Tok_t operator;
    ExprTree *additive_expr, *mult_expr, *add_expr;

    mult_expr = parse_multiplicative_expr(tok_l, &t);

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     * NOTE: When operator is + or - the above is redundant/excess work. A possible  *
     *       future optimization is to make the token list doubly-linked and parse   *
     *       starting from the end. It would be interesting to benchmark the fix.    *
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    operator = t->token;
    switch(operator) {
        case TOK_ADD:
        case TOK_SUB:
            add_expr = parse_additive_expr(tok_l, &t);
            t2 = match_token(t, operator);
            mult_expr = parse_multiplicative_expr(t2, &t3);
            break;
        default:
            *out_tl = t;
            return mult_expr;
    }

    additive_expr = malloc(sizeof(ExprTree));
    additive_expr->expr = Binop;
    additive_expr->value.binop = (operator == TOK_ADD)? Add : Sub;
    additive_expr->left = add_expr;
    additive_expr->right = mult_expr;

    *out_tl = t3;
    return additive_expr;
}

static ExprTree *parse_multiplicative_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    Tok_t operator;
    ExprTree *multiplicative_expr, *app_expr, *mult_expr;

    app_expr = parse_application_expr(tok_l, &t);

    operator = t->token;
    switch(operator) {
        case TOK_MULT:
        case TOK_DIV:
            mult_expr = parse_multiplicative_expr(tok_l, &t);
            t2 = match_token(t, operator);
            app_expr = parse_application_expr(t2, &t3);
            break;
        default:
            *out_tl = t;
            return app_expr;
    }

    multiplicative_expr = malloc(sizeof(ExprTree));
    multiplicative_expr->expr = Binop;
    multiplicative_expr->value.binop = (operator == TOK_MULT)? Mult : Div;
    multiplicative_expr->left = mult_expr;
    multiplicative_expr->right = app_expr;

    *out_tl = t3;
    return multiplicative_expr;
}

static ExprTree *parse_application_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3, *t4;
    ExprTree *application_expr, *primary_expr, *arg_expr;
    char *id, *id_cpy;


    switch(tok_l->token) {
        case TOK_ID:
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
            /* The above is to ignore a warning (this time only) for implicit fall through caused by this case */
            if (tok_l->next->token == TOK_LPAREN) {
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
            /* TODO: Consider printing original input str (convert back) instead of token list */
            char *tl_str = token_list_to_str(tok_l);
            fprintf(stderr, 
                    "parser: failed to find primary expression with remaining tokens:\n%s\n",
                    tl_str);
            free(tl_str);
            exit(EXIT_FAILURE);
        }
    }
    
    t = match_token(tok_l, TOK_ID);

    id = tok_l->value.id;
    id_cpy = malloc(strlen(id) + 1);
    strcpy(id_cpy, id);

    t2 = match_token(t, TOK_LPAREN);
    arg_expr = parse_arg_expr(t2, &t3);
    t4 = match_token(t3, TOK_RPAREN);

    application_expr = malloc(sizeof(ExprTree));
    application_expr->expr = Application;
    application_expr->value.id = id_cpy;
    application_expr->left = NULL;
    application_expr->right = arg_expr;

    *out_tl = t4;
    return application_expr;
}

static ExprTree *parse_arg_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *argument_expr, *add_expr, *arg_expr;

    add_expr = parse_additive_expr(tok_l, &t);

    if (t->token == TOK_COMMA) {
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

static ExprTree *parse_primary_expr(TokenList *tok_l, TokenList **out_tl) {
    TokenList *t, *t2, *t3;
    ExprTree *p_expr, *add_expr;
    char *id, *id_cpy;

    p_expr = malloc(sizeof(ExprTree));

    switch(tok_l->token) {
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
            char *tl_str = token_list_to_str(tok_l);
            free(p_expr);

            fprintf(stderr, 
                    "parser: unrecognized primary expression with remaining tokens:\n%s\n",
                    tl_str);
            free(tl_str);
            exit(EXIT_FAILURE);
        }
    }

    p_expr->left = NULL;
    p_expr->right = NULL;

    return p_expr;
}

static void free_tree_node(ExprTree *tree) {
    if (tree->expr == ID) {
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
