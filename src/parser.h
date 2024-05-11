#ifndef Parser_h
#define Parser_h

#include "lexer.h"

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
        Operator_t binop;
    } value;
    struct expr_tree *left;
    struct expr_tree *right;
} ExprTree;

ExprTree *parse(TokenList *tok_l);

#endif
