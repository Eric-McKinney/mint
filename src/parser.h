#ifndef Parser_h
#define Parser_h

#include "parser.h"

typedef enum {
    Add,
    Sub,
    Mult,
    Div,
    Exp
} Operator_t;

typedef enum {
    Int,
    Float,
    ID,
    Fun,
    Binop
} Expr_t;

typedef struct expr_tree {
    Expr_t expr;
    union {
        int i;
        double d;
        char *id;
    } value;
    struct expr_tree *left;
    struct expr_tree *right;
} ExprTree;

ExprTree *parse_expr(TokenList *tok_l);

#endif
